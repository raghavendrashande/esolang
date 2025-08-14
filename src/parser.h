#pragma once
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <unordered_map>
#include "tokenization.h"
using namespace std;

enum class ExprKind { Int, Str, Var, Unary, Binary, Grouping, Assign };
enum class StmtKind { Yeet, ExprStmt, Block, If, LoopDoIt, VarDecl, Print };

struct Expr {
    ExprKind kind;
    long long int_lit;
    string str_lit;
    string var_name;
    struct { TokenType op; unique_ptr<Expr> operand; } unary;
    struct { TokenType op; unique_ptr<Expr> left; unique_ptr<Expr> right; } binary;
    struct { unique_ptr<Expr> inner; } group;
    struct { string name; unique_ptr<Expr> value; } assign;
};

struct Stmt {
    StmtKind kind;
    struct { unique_ptr<Expr> expr; } exit;
    struct { unique_ptr<Expr> expr; } exprstmt;
    struct { vector<unique_ptr<Stmt>> stmts; } block;
    struct { unique_ptr<Expr> cond; vector<unique_ptr<Stmt>> then_stmts; vector<unique_ptr<Stmt>> else_stmts; } ifs;
    struct { unique_ptr<Expr> count; vector<unique_ptr<Stmt>> body; } loop;
    struct { string name; unique_ptr<Expr> value; } vardecl;
    struct { unique_ptr<Expr> expr; } print;
};

struct Program { vector<unique_ptr<Stmt>> body; };

class Parser {
public:
    inline explicit Parser(vector<Token> t) : toks(move(t)) {}
    optional<Program> parse() {
        Program prog;
        while (!is_at_end()) {
            auto s = parse_stmt();
            if (!s) return {};
            prog.body.push_back(move(*s));
        }
        return prog;
    }
private:
    vector<Token> toks;
    size_t i = 0;

    bool is_at_end() const { return peek().type == TokenType::Eof; }
    const Token& peek() const { return toks[i]; }
    const Token& prev() const { return toks[i-1]; }
    const Token& advance() { if (!is_at_end()) i++; return prev(); }
    bool check(TokenType t) const { return !is_at_end() && peek().type == t; }
    bool match(initializer_list<TokenType> ts) { for (auto t: ts) if (check(t)) { advance(); return true; } return false; }
    bool consume(TokenType t) { if (check(t)) { advance(); return true; } return false; }

    optional<unique_ptr<Stmt>> parse_stmt() {
        if (match({TokenType::Yeet})) {
            auto e = parse_expr();
            if (!e) return {};
            if (!consume(TokenType::Semicolon)) return {};
            auto s = make_unique<Stmt>();
            s->kind = StmtKind::Yeet;
            s->exit.expr = move(*e);
            return s;
        }
        if (match({TokenType::Is})) {
            if (!consume(TokenType::LParen)) return {};
            auto cond = parse_expr();
            if (!cond) return {};
            if (!consume(TokenType::RParen)) return {};
            auto thenb = parse_block();
            if (!thenb) return {};
            if (!match({TokenType::Else})) return {};
            auto elseb = parse_block();
            if (!elseb) return {};
            auto s = make_unique<Stmt>();
            s->kind = StmtKind::If;
            s->ifs.cond = move(*cond);
            s->ifs.then_stmts = move((*thenb)->block.stmts);
            s->ifs.else_stmts = move((*elseb)->block.stmts);
            return s;
        }
        if (match({TokenType::DoIt})) {
            if (!consume(TokenType::LParen)) return {};
            auto cnt = parse_expr();
            if (!cnt) return {};
            if (!consume(TokenType::RParen)) return {};
            auto body = parse_block();
            if (!body) return {};
            auto s = make_unique<Stmt>();
            s->kind = StmtKind::LoopDoIt;
            s->loop.count = move(*cnt);
            s->loop.body = move((*body)->block.stmts);
            return s;
        }
        if (match({TokenType::Imagine})) {
            if (!check(TokenType::Identifier)) return {};
            string name = peek().lexeme.value();
            advance();
            if (!consume(TokenType::Assign)) return {};
            auto val = parse_expr();
            if (!val) return {};
            if (!consume(TokenType::Semicolon)) return {};
            auto s = make_unique<Stmt>();
            s->kind = StmtKind::VarDecl;
            s->vardecl.name = name;
            s->vardecl.value = move(*val);
            return s;
        }
        if (match({TokenType::Print})) {
            if (!consume(TokenType::LParen)) return {};
            auto e = parse_expr();
            if (!e) return {};
            if (!consume(TokenType::RParen)) return {};
            if (!consume(TokenType::Semicolon)) return {};
            auto s = make_unique<Stmt>();
            s->kind = StmtKind::Print;
            s->print.expr = move(*e);
            return s;
        }
        if (check(TokenType::LBrace)) {
            return parse_block();
        }
        auto e = parse_expr();
        if (!e) return {};
        if (!consume(TokenType::Semicolon)) return {};
        auto s = make_unique<Stmt>();
        s->kind = StmtKind::ExprStmt;
        s->exprstmt.expr = move(*e);
        return s;
    }

    optional<unique_ptr<Stmt>> parse_block() {
        if (!consume(TokenType::LBrace)) return {};
        vector<unique_ptr<Stmt>> stmts;
        while (!check(TokenType::RBrace) && !is_at_end()) {
            auto s = parse_stmt();
            if (!s) return {};
            stmts.push_back(move(*s));
        }
        if (!consume(TokenType::RBrace)) return {};
        auto blk = make_unique<Stmt>();
        blk->kind = StmtKind::Block;
        blk->block.stmts = move(stmts);
        return blk;
    }

    optional<unique_ptr<Expr>> parse_expr() { return parse_assignment(); }
    optional<unique_ptr<Expr>> parse_assignment() {
        auto left = parse_equality();
        if (!left) return {};
        if (match({TokenType::Assign})) {
            if (left.value()->kind != ExprKind::Var) return {};
            string name = left.value()->var_name;
            auto value = parse_assignment();
            if (!value) return {};
            auto e = make_unique<Expr>();
            e->kind = ExprKind::Assign;
            e->assign.name = name;
            e->assign.value = move(*value);
            return e;
        }
        return left;
    }
    optional<unique_ptr<Expr>> parse_equality() {
        auto left = parse_relational();
        if (!left) return {};
        while (match({TokenType::EqualEqual, TokenType::BangEqual})) {
            TokenType op = prev().type;
            auto right = parse_relational();
            if (!right) return {};
            left = make_bin(move(*left), op, move(*right));
        }
        return left;
    }
    optional<unique_ptr<Expr>> parse_relational() {
        auto left = parse_additive();
        if (!left) return {};
        while (match({TokenType::Less, TokenType::LessEqual, TokenType::Greater, TokenType::GreaterEqual})) {
            TokenType op = prev().type;
            auto right = parse_additive();
            if (!right) return {};
            left = make_bin(move(*left), op, move(*right));
        }
        return left;
    }
    optional<unique_ptr<Expr>> parse_additive() {
        auto left = parse_multiplicative();
        if (!left) return {};
        while (match({TokenType::Plus, TokenType::Minus})) {
            TokenType op = prev().type;
            auto right = parse_multiplicative();
            if (!right) return {};
            left = make_bin(move(*left), op, move(*right));
        }
        return left;
    }
    optional<unique_ptr<Expr>> parse_multiplicative() {
        auto left = parse_unary();
        if (!left) return {};
        while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
            TokenType op = prev().type;
            auto right = parse_unary();
            if (!right) return {};
            left = make_bin(move(*left), op, move(*right));
        }
        return left;
    }
    optional<unique_ptr<Expr>> parse_unary() {
        if (match({TokenType::Minus})) {
            TokenType op = prev().type;
            auto operand = parse_unary();
            if (!operand) return {};
            auto e = make_unique<Expr>();
            e->kind = ExprKind::Unary;
            e->unary.op = op;
            e->unary.operand = move(*operand);
            return e;
        }
        return parse_primary();
    }
    optional<unique_ptr<Expr>> parse_primary() {
        if (match({TokenType::Int})) {
            auto e = make_unique<Expr>();
            e->kind = ExprKind::Int;
            e->int_lit = stoll(prev().lexeme.value());
            return e;
        }
        if (match({TokenType::String})) {
            auto e = make_unique<Expr>();
            e->kind = ExprKind::Str;
            e->str_lit = prev().lexeme.value();
            return e;
        }
        if (match({TokenType::Identifier})) {
            auto e = make_unique<Expr>();
            e->kind = ExprKind::Var;
            e->var_name = prev().lexeme.value();
            return e;
        }
        if (match({TokenType::LParen})) {
            auto inner = parse_expr();
            if (!inner) return {};
            if (!consume(TokenType::RParen)) return {};
            auto e = make_unique<Expr>();
            e->kind = ExprKind::Grouping;
            e->group.inner = move(*inner);
            return e;
        }
        return {};
    }
    unique_ptr<Expr> make_bin(unique_ptr<Expr> l, TokenType op, unique_ptr<Expr> r) {
        auto e = make_unique<Expr>();
        e->kind = ExprKind::Binary;
        e->binary.left = move(l);
        e->binary.right = move(r);
        e->binary.op = op;
        return e;
    }
};
