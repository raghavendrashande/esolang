#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <stdexcept>
using namespace std;

enum class TokenType {
    Is, Else, DoIt, Yeet, Imagine, Print,
    Identifier, String, Int,
    Plus, Minus, Star, Slash, Percent,
    EqualEqual, BangEqual, Less, LessEqual, Greater, GreaterEqual,
    Assign,
    LParen, RParen, LBrace, RBrace, Semicolon,
    Eof
};

struct Token {
    TokenType type;
    optional<string> lexeme;
};

class Tokenizer {
public:
    inline explicit Tokenizer(string s) : src(move(s)) {}
    vector<Token> tokenize() {
        vector<Token> out;
        while (!is_at_end()) {
            skip_ws();
            if (is_at_end()) break;
            char c = peek();
            if (isdigit(c)) { out.push_back(Token{TokenType::Int, read_number()}); continue; }
            if (isalpha(c) || c == '_') {
                string id = read_ident();
                if (id == "is") out.push_back(Token{TokenType::Is, {}});
                else if (id == "else") out.push_back(Token{TokenType::Else, {}});
                else if (id == "doit") out.push_back(Token{TokenType::DoIt, {}});
                else if (id == "yeet") out.push_back(Token{TokenType::Yeet, {}});
                else if (id == "imagine") out.push_back(Token{TokenType::Imagine, {}});
                else if (id == "print") out.push_back(Token{TokenType::Print, {}});
                else out.push_back(Token{TokenType::Identifier, id});
                continue;
            }
            if (c == '"') { out.push_back(Token{TokenType::String, read_string()}); continue; }
            if (c == '+') { advance(); out.push_back(Token{TokenType::Plus, {}}); continue; }
            if (c == '-') { advance(); out.push_back(Token{TokenType::Minus, {}}); continue; }
            if (c == '*') { advance(); out.push_back(Token{TokenType::Star, {}}); continue; }
            if (c == '/') { advance(); out.push_back(Token{TokenType::Slash, {}}); continue; }
            if (c == '%') { advance(); out.push_back(Token{TokenType::Percent, {}}); continue; }
            if (c == '(') { advance(); out.push_back(Token{TokenType::LParen, {}}); continue; }
            if (c == ')') { advance(); out.push_back(Token{TokenType::RParen, {}}); continue; }
            if (c == '{') { advance(); out.push_back(Token{TokenType::LBrace, {}}); continue; }
            if (c == '}') { advance(); out.push_back(Token{TokenType::RBrace, {}}); continue; }
            if (c == ';') { advance(); out.push_back(Token{TokenType::Semicolon, {}}); continue; }
            if (c == '!') { advance(); if (match('=')) out.push_back(Token{TokenType::BangEqual, {}}); else fail(); continue; }
            if (c == '<') { advance(); if (match('=')) out.push_back(Token{TokenType::LessEqual, {}}); else out.push_back(Token{TokenType::Less, {}}); continue; }
            if (c == '>') { advance(); if (match('=')) out.push_back(Token{TokenType::GreaterEqual, {}}); else out.push_back(Token{TokenType::Greater, {}}); continue; }
            if (c == '=') { advance(); if (match('=')) out.push_back(Token{TokenType::EqualEqual, {}}); else out.push_back(Token{TokenType::Assign, {}}); continue; }
            fail();
        }
        out.push_back(Token{TokenType::Eof, {}});
        return out;
    }
private:
    string src;
    size_t i = 0;
    bool is_at_end() const { return i >= src.size(); }
    char peek() const { return src[i]; }
    char advance() { return src[i++]; }
    bool match(char ch) { if (!is_at_end() && src[i] == ch) { i++; return true; } return false; }
    void skip_ws() { while (!is_at_end() && isspace((unsigned char)src[i])) i++; }
    [[noreturn]] void fail() { throw runtime_error("lex error"); }
    string read_number() { size_t s = i; while (!is_at_end() && isdigit((unsigned char)src[i])) i++; return src.substr(s, i - s); }
    string read_ident() { size_t s = i; while (!is_at_end() && (isalnum((unsigned char)src[i]) || src[i] == '_')) i++; return src.substr(s, i - s); }
    string read_string() { advance(); size_t s = i; while (!is_at_end() && peek() != '"') i++; string v = src.substr(s, i - s); if (is_at_end()) fail(); advance(); return v; }
};
