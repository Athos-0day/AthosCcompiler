#pragma once
#include <string>
#include <vector>

enum class Token {
    IDENTIFIER, CONSTANT, INT, VOID, RETURN,
    OPARENTHESIS, CPARENTHESIS, OBRACE, CBRACE, SEMICOLON, SKIP, COMMENT, ML_COMMENT, MISMATCH, COMPLEMENT,
    NEGATION, DECREMENT, ADDITION, MULTIPLICATION, DIVISION, REMAINDER, NOT, AND, OR, EQUAL, NOTEQUAL,
    LESS, GREATER, LESSEQ, GREATEREQ, ASSIGN, IF, ELSE, COLON, QUESTION_MARK, DO, WHILE, FOR, BREAK, CONTINUE
};

/**
 * @struct Lex
 * @brief Represents a lexical token.
 * 
 * Contains the raw token string, the token type, its position (order) in the input stream,
 * and the line number where the token appears in the source file.
 */
struct Lex {
    std::string word;
    Token token;
    int position;
    int line;           
};

std::vector<Lex> lexer(const std::string& filename, bool verbose = true);
std::string tokenToString(Token t);

