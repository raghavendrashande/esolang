#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "parser.h"
using namespace std;

class Generator {
public:
    inline explicit Generator(const Program& prog) : p(prog) {}
    string generate() {
        collect();
        o.str(string());
        o.clear();
        o << "section .data\n";
        o << "newline db 10\n";
        for (size_t i = 0; i < strings.size(); ++i) {
            o << "str" << i << " db ";
            const string& s = strings[i];
            bool first = true;
            o << "'";
            for (char c : s) {
                if (c == '\'') { o << "'"; o << "'"; }
                else { o << c; }
            }
            o << "'\n";
            o << "str" << i << "_len equ $-str" << i << "\n";
        }
        o << "section .bss\n";
        o << "numbuf resb 64\n";
        for (size_t i = 0; i < vars.size(); ++i) {
            o << "var" << i << " resq 1\n";
        }
        o << "section .text\n";
        o << "global _start\n";
        o << "_start:\n";
        gen_block(p.body);
        o << "    mov rax, 60\n";
        o << "    mov rdi, 0\n";
        o << "    syscall\n";
        return o.str();
    }
private:
    const Program& p;
    stringstream o;
    unordered_map<string,int> var_index;
    vector<string> vars;
    vector<string> strings;
    unordered_map<string,int> str_index;
    int label_id = 0;

    int new_label() { return ++label_id; }

    void collect_expr(const Expr& e) {
        switch (e.kind) {
            case ExprKind::Var:
                ensure_var(e.var_name);
                break;
            case ExprKind::Assign:
                ensure_var(e.assign.name);
                collect_expr(*e.assign.value);
                break;
            case ExprKind::Unary:
                collect_expr(*e.unary.operand);
                break;
            case ExprKind::Binary:
                collect_expr(*e.binary.left);
                collect_expr(*e.binary.right);
                break;
            case ExprKind::Grouping:
                collect_expr(*e.group.inner);
                break;
            case ExprKind::Str:
                ensure_str(e.str_lit);
                break;
            default:
                break;
        }
    }
    void collect_stmt(const Stmt& s) {
        switch (s.kind) {
            case StmtKind::VarDecl:
                ensure_var(s.vardecl.name);
                collect_expr(*s.vardecl.value);
                break;
            case StmtKind::Yeet:
                collect_expr(*s.exit.expr);
                break;
            case StmtKind::ExprStmt:
                collect_expr(*s.exprstmt.expr);
                break;
            case StmtKind::If:
                collect_expr(*s.ifs.cond);
                for (auto& t : s.ifs.then_stmts) collect_stmt(*t);
                for (auto& e : s.ifs.else_stmts) collect_stmt(*e);
                break;
            case StmtKind::LoopDoIt:
                collect_expr(*s.loop.count);
                for (auto& b : s.loop.body) collect_stmt(*b);
                break;
            case StmtKind::Print:
                collect_expr(*s.print.expr);
                break;
            case StmtKind::Block:
                for (auto& b : s.block.stmts) collect_stmt(*b);
                break;
        }
    }
    void collect() {
        for (auto& s : p.body) collect_stmt(*s);
    }
    void ensure_var(const string& name) {
        if (!var_index.count(name)) {
            int idx = (int)vars.size();
            var_index[name] = idx;
            vars.push_back(name);
        }
    }
    void ensure_str(const string& s) {
        if (!str_index.count(s)) {
            int idx = (int)strings.size();
            str_index[s] = idx;
            strings.push_back(s);
        }
    }

    void gen_block(const vector<unique_ptr<Stmt>>& stmts) {
        for (auto& s : stmts) gen_stmt(*s);
    }

    void gen_stmt(const Stmt& s) {
        switch (s.kind) {
            case StmtKind::Yeet: {
                gen_expr(*s.exit.expr);
                o << "    mov rdi, rax\n";
                o << "    mov rax, 60\n";
                o << "    syscall\n";
                break;
            }
            case StmtKind::ExprStmt: {
                gen_expr(*s.exprstmt.expr);
                break;
            }
            case StmtKind::Block: {
                gen_block(s.block.stmts);
                break;
            }
            case StmtKind::If: {
                int L_else = new_label();
                int L_end = new_label();
                gen_expr(*s.ifs.cond);
                o << "    cmp rax, 0\n";
                o << "    je .L" << L_else << "\n";
                gen_block(s.ifs.then_stmts);
                o << "    jmp .L" << L_end << "\n";
                o << ".L" << L_else << ":\n";
                gen_block(s.ifs.else_stmts);
                o << ".L" << L_end << ":\n";
                break;
            }
            case StmtKind::LoopDoIt: {
                int L_top = new_label();
                int L_end = new_label();
                gen_expr(*s.loop.count);
                o << "    mov r10, rax\n";
                o << "    cmp r10, 0\n";
                o << "    jle .L" << L_end << "\n";
                o << ".L" << L_top << ":\n";
                gen_block(s.loop.body);
                o << "    dec r10\n";
                o << "    jnz .L" << L_top << "\n";
                o << ".L" << L_end << ":\n";
                break;
            }
            case StmtKind::VarDecl: {
                gen_expr(*s.vardecl.value);
                int idx = var_index[s.vardecl.name];
                o << "    mov [var" << idx << "], rax\n";
                break;
            }
            case StmtKind::Print: {
                gen_print(*s.print.expr);
                break;
            }
        }
    }

    void gen_print(const Expr& e) {
        if (e.kind == ExprKind::Str) {
            int si = str_index[e.str_lit];
            o << "    mov rax, 1\n";
            o << "    mov rdi, 1\n";
            o << "    mov rsi, str" << si << "\n";
            o << "    mov rdx, str" << si << "_len\n";
            o << "    syscall\n";
            o << "    mov rax, 1\n";
            o << "    mov rdi, 1\n";
            o << "    mov rsi, newline\n";
            o << "    mov rdx, 1\n";
            o << "    syscall\n";
        } else {
            gen_expr(e);
            int L = new_label();
            o << "    mov rbx, 10\n";
            o << "    mov rsi, numbuf+64\n";
            o << "    xor rcx, rcx\n";
            o << "    cmp rax, 0\n";
            o << "    jge .Lpos" << L << "\n";
            o << "    neg rax\n";
            o << "    mov rcx, 1\n";
            o << ".Lpos" << L << ":\n";
            o << "    xor rdx, rdx\n";
            o << ".Lloop" << L << ":\n";
            o << "    xor rdx, rdx\n";
            o << "    div rbx\n";
            o << "    dec rsi\n";
            o << "    add rdx, '0'\n";
            o << "    mov [rsi], dl\n";
            o << "    test rax, rax\n";
            o << "    jnz .Lloop" << L << "\n";
            o << "    cmp rcx, 0\n";
            o << "    je .Lnosign" << L << "\n";
            o << "    dec rsi\n";
            o << "    mov byte [rsi], '-'\n";
            o << ".Lnosign" << L << ":\n";
            o << "    mov rax, 1\n";
            o << "    mov rdi, 1\n";
            o << "    mov rdx, numbuf+64\n";
            o << "    sub rdx, rsi\n";
            o << "    mov rsi, rsi\n";
            o << "    syscall\n";
            o << "    mov rax, 1\n";
            o << "    mov rdi, 1\n";
            o << "    mov rsi, newline\n";
            o << "    mov rdx, 1\n";
            o << "    syscall\n";
        }
    }

    void gen_expr(const Expr& e) {
        switch (e.kind) {
            case ExprKind::Int: {
                o << "    mov rax, " << e.int_lit << "\n";
                break;
            }
            case ExprKind::Str: {
                int si = str_index[e.str_lit];
                o << "    lea rax, [rel str" << si << "]\n";
                break;
            }
            case ExprKind::Var: {
                int idx = var_index[e.var_name];
                o << "    mov rax, [var" << idx << "]\n";
                break;
            }
            case ExprKind::Assign: {
                gen_expr(*e.assign.value);
                {
                    int idx = var_index[e.assign.name];
                    o << "    mov [var" << idx << "], rax\n";
                }
                break;
            }
            case ExprKind::Unary: {
                gen_expr(*e.unary.operand);
                if (e.unary.op == TokenType::Minus) {
                    o << "    neg rax\n";
                }
                break;
            }
            case ExprKind::Binary: {
                gen_expr(*e.binary.left);
                o << "    push rax\n";
                gen_expr(*e.binary.right);
                o << "    mov rbx, rax\n";
                o << "    pop rcx\n";
                switch (e.binary.op) {
                    case TokenType::Plus:
                        o << "    add rcx, rbx\n";
                        o << "    mov rax, rcx\n";
                        break;
                    case TokenType::Minus:
                        o << "    sub rcx, rbx\n";
                        o << "    mov rax, rcx\n";
                        break;
                    case TokenType::Star:
                        o << "    mov rax, rcx\n";
                        o << "    imul rax, rbx\n";
                        break;
                    case TokenType::Slash:
                        o << "    mov rax, rcx\n";
                        o << "    cqo\n";
                        o << "    idiv rbx\n";
                        break;
                    case TokenType::Percent:
                        o << "    mov rax, rcx\n";
                        o << "    cqo\n";
                        o << "    idiv rbx\n";
                        o << "    mov rax, rdx\n";
                        break;
                    case TokenType::EqualEqual:
                        o << "    cmp rcx, rbx\n";
                        o << "    mov rax, 0\n";
                        o << "    sete al\n";
                        break;
                    case TokenType::BangEqual:
                        o << "    cmp rcx, rbx\n";
                        o << "    mov rax, 0\n";
                        o << "    setne al\n";
                        break;
                    case TokenType::Less:
                        o << "    cmp rcx, rbx\n";
                        o << "    mov rax, 0\n";
                        o << "    setl al\n";
                        break;
                    case TokenType::LessEqual:
                        o << "    cmp rcx, rbx\n";
                        o << "    mov rax, 0\n";
                        o << "    setle al\n";
                        break;
                    case TokenType::Greater:
                        o << "    cmp rcx, rbx\n";
                        o << "    mov rax, 0\n";
                        o << "    setg al\n";
                        break;
                    case TokenType::GreaterEqual:
                        o << "    cmp rcx, rbx\n";
                        o << "    mov rax, 0\n";
                        o << "    setge al\n";
                        break;
                    default:
                        break;
                }
                break;
            }
            case ExprKind::Grouping: {
                gen_expr(*e.group.inner);
                break;
            }
        }
    }
};
