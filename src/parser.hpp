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
 *  ‹exp›       ::= <factor> | <exp> <binop> <exp>
 *  <factor>    ::= <int> | <unop> <factor> | "(" <exp> ")"
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

    /**
     * @brief Returns the precedence level of a binary operator token.
     *
     * Used during expression parsing to enforce operator precedence.
     *
     * Precedences:
     *   - '+' / '-' : 45
     *   - '*' / '/' / '%' : 50
     *
     * @param token The operator token.
     * @return Integer precedence value, or -1 if not a binary operator.
     */
    int getPrecedence(Token token);

    /**
     * @brief Converts a token representing a binary operator into its AST equivalent.
     *
     * @param token Token to convert (must be a binary operator).
     * @return Corresponding BinaryOpast enum value.
     * @throws std::runtime_error If the token is not a supported binary operator.
     */
    BinaryOpast tokenToBinaryOp(Token token);

        /**
     * @brief Converts a token representing a unary operator into its AST equivalent.
     *
     * Recognized unary operators are:
     *   - '~' (bitwise complement) → UnaryOpast::COMPLEMENT
     *   - '-' (negation) → UnaryOpast::NEGATE
     *   - '!' (logical not) → UnaryOpast::NOT
     *
     * This is used when parsing unary expressions to build the correct AST node.
     *
     * @param token Token to convert (must be a unary operator).
     * @return Corresponding UnaryOpast enum value.
     * @throws std::runtime_error If the token is not a supported unary operator.
     */
    UnaryOpast tokenToUnaryOp(Token token);

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
     * @brief Parses a ‹block-item› inside a function body.
     *
     * A block item can currently be:
     *   - a declaration
     *   - a statement
     *
     * Future extensions might allow variable declarations or compound statements.
     *
     * @return Unique pointer to a BlockItem AST node.
     * @throws std::runtime_error If the block item is not recognized.
     */
    std::unique_ptr<BlockItem> parseBlockItem();

    /**
     * @brief Parses a ‹statement› according to the grammar.
     * @return Unique pointer to the ReturnStatement AST node.
     * @throws std::runtime_error If parsing fails.
     */
    std::unique_ptr<Statement> parseStatement();

    /**
     * @brief Parses an ‹exp› according to the grammar with default precedence.
     * @return Unique pointer to the Expression AST node.
     * @throws std::runtime_error If parsing fails.
     */
    std::unique_ptr<Expression> parseExpression();

    /**
     * @brief Parses an ‹exp› using precedence climbing algorithm.
     *
     * Used internally to correctly parse binary operations with varying precedence.
     *
     * @param minPrecedence Minimum precedence level required to continue parsing.
     * @return Unique pointer to the Expression AST node.
     */
    std::unique_ptr<Expression> parseExpression(int minPrecedence);

    /**
     * @brief Parses a <factor> according to the grammar.
     * @return Unique pointer to the Expression AST node.
     * @throws std::runtime_error If parsing fails.
     */
    std::unique_ptr<Expression> parseFactor();

    /**
     * @brief Ensures the current token matches the expected type, consumes it, and returns it.
     *
     * If the token does not match the expected type, this function throws a syntax error
     * with a custom error message and token position details.
     *
     * @param expected The expected token type.
     * @param errorMsg The error message to display if the token does not match.
     * @return The consumed token.
     * @throws std::runtime_error If the current token is not of the expected type.
     */
    Lex expect(Token expected, const std::string& errorMsg);
};

#endif // PARSER_HPP
