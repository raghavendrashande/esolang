#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include "tokenization.h"
#include "parser.h"
#include "generation.h"
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "usage: bl <input.bl>\n";
        return EXIT_FAILURE;
    }
    string contents;
    {
        ifstream in(argv[1], ios::in | ios::binary);
        if (!in) {
            cerr << "cannot open input\n";
            return EXIT_FAILURE;
        }
        stringstream ss;
        ss << in.rdbuf();
        contents = ss.str();
    }
    vector<Token> toks;
    try {
        Tokenizer tz(contents);
        toks = tz.tokenize();
    } catch (const exception& e) {
        cerr << "tokenize error\n";
        return EXIT_FAILURE;
    }
    Parser parser(move(toks));
    auto prog = parser.parse();
    if (!prog.has_value()) {
        cerr << "parse error\n";
        return EXIT_FAILURE;
    }
    Generator gen(prog.value());
    {
        ofstream out("out.asm", ios::out | ios::trunc);
        out << gen.generate();
    }
    int a = system("nasm -felf64 -o out.o out.asm");
    if (a != 0) { cerr << "assemble failed\n"; return EXIT_FAILURE; }
    int l = system("ld -o out out.o");
    if (l != 0) { cerr << "link failed\n"; return EXIT_FAILURE; }
    return EXIT_SUCCESS;
}
