/**
 * @file parser.hpp
 * @brief Recursive descent parser for a simple C-like language subset.
 *
 * This parser takes tokens produced by the lexer and builds an Abstract Syntax Tree (AST)
 * according to the following grammar:
 *
 *  ‹program›   ::= ‹function›
 *  ‹function›  ::= "int" ‹identifier› "(" "void" ")" "{" ‹statement› "}"
 *  ‹statement› ::= "return" ‹exp› ";"
 *  ‹exp›       ::= <int> | <unop> <exp> | "(" <exp> ")"
 *
 * It supports verbose logging for debugging and throws detailed exceptions
 * on syntax errors including line information.
 */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

#include "lexer.hpp"
#include "ast.hpp"

/**
 * @class Parser
 * @brief Recursive descent parser that builds an AST from tokens.
 *
 * Provides methods to parse a program, function, statement, and expression
 * according to the specified grammar.
 */
class Parser {
private:
    std::vector<Lex> tokens;  /**< List of tokens to parse */
    size_t current = 0;       /**< Current position in the tokens vector */
    bool verbose;             /**< Verbose mode flag to enable debug logs */

    /**
     * @brief Throws a runtime error with a detailed parse error message.
     * @param message Description of the parse error.
     * @param token The token at which the error was detected.
     * @throws std::runtime_error Always throws with formatted error message.
     */
    [[noreturn]] void error(const std::string& message, const Lex& token);

    /**
     * @brief Logs messages to stdout if verbose mode is enabled.
     * @param message Message to log.
     */
    void log(const std::string& message);

public:
    /**
     * @brief Constructs the parser with tokens and optional verbose mode.
     * @param t Vector of tokens to parse.
     * @param verboseMode Enables verbose output if true (default false).
     */
    Parser(const std::vector<Lex>& t, bool verboseMode = false);

    /**
     * @brief Returns the current token without consuming it.
     * @return The current token or a MISMATCH token if at end.
     */
    Lex peek();

    /**
     * @brief Consumes and returns the current token.
     * @return The consumed token or a MISMATCH token if at end.
     */
    Lex advance();

    /**
     * @brief Checks if the current token matches the given type and consumes it.
     * @param t Token type to match.
     * @return True if matched and consumed, false otherwise.
     */
    bool match(Token t);

    /**
     * @brief Parses a ‹program› according to the grammar.
     * @return Unique pointer to the Program AST node.
     * @throws std::runtime_error If parsing fails.
     */
    std::unique_ptr<Program> parseProgram();

    /**
     * @brief Parses a ‹function› according to the grammar.
     * @return Unique pointer to the Function AST node.
     * @throws std::runtime_error If parsing fails.
     */
    std::unique_ptr<Function> parseFunction();

    /**
     * @brief Parses a ‹statement› according to the grammar.
     * @return Unique pointer to the ReturnStatement AST node.
     * @throws std::runtime_error If parsing fails.
     */
    std::unique_ptr<ReturnStatement> parseStatement();

    /**
     * @brief Parses an ‹exp› according to the grammar.
     * @return Unique pointer to the Expression AST node.
     * @throws std::runtime_error If parsing fails.
     */
    std::unique_ptr<Expression> parseExpression();
};

#endif // PARSER_HPP
