#pragma once
#include <string>
#include <vector>

enum class Token {
    IDENTIFIER, CONSTANT, INT, VOID, RETURN,
    OPARENTHESIS, CPARENTHESIS, OBRACE, CBRACE, SEMICOLON, SKIP, COMMENT, ML_COMMENT, MISMATCH, COMPLEMENT,
    NEGATION, DECREMENT, ADDITION, MULTIPLICATION, DIVISION, REMAINDER, NOT, AND, OR, EQUAL, NOTEQUAL,
    LESS, GREATER, LESSEQ, GREATEREQ
};

struct Lex {
    std::string word;
    Token token;
    int position;
    int line;           
};

std::vector<Lex> lexer(const std::string& filename, bool verbose = true);
std::string tokenToString(Token t);

