#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <memory>
#include "lexer.hpp"

/**
 * @brief Base class for all nodes in the Abstract Syntax Tree (AST).
 */
class ASTNode {
public:
    /**
     * @brief Virtual destructor to allow proper cleanup of derived classes.
     */
    virtual ~ASTNode() = default;
};

/**
 * @brief Represents an expression node.
 * 
 * Supports:
 * - Integer constant: ‹exp› ::= <int>
 * - Unary expression: ‹exp› ::= <unop> ‹exp›
 * - Parenthesized expression: ‹exp› ::= "(" ‹exp› ")"
 */
class Expression : public ASTNode {
public:
    // Variant data
    int value; ///< Used only if this is a constant
    Token op; ///< Used only if this is a unary expression
    std::unique_ptr<Expression> operand; ///< Operand of unary expression

    /**
     * @brief Constructs an integer literal expression.
     * 
     * @param v The integer value.
     */
    Expression(int v) : value(v), op(Token::MISMATCH), operand(nullptr) {}

    /**
     * @brief Constructs a unary expression.
     * 
     * @param unaryOp The unary operator (e.g., NEGATION, COMPLEMENT).
     * @param expr The operand expression.
     */
    Expression(Token unaryOp, std::unique_ptr<Expression> expr)
        : value(0), op(unaryOp), operand(std::move(expr)) {}
};

/**
 * @brief Represents a return statement node.
 * 
 * This corresponds to the grammar rule: ‹statement› ::= "return" ‹exp› ";"
 */
class ReturnStatement : public ASTNode {
public:
    std::unique_ptr<Expression> expression; ///< Pointer to the expression being returned

    /**
     * @brief Constructs a ReturnStatement node.
     * 
     * @param expr The expression to be returned.
     */
    ReturnStatement(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
};

/**
 * @brief Represents a function definition.
 * 
 * This corresponds to the grammar rule:
 * ‹function› ::= "int" ‹identifier› "(" "void" ")" "{" ‹statement› "}"
 */
class Function : public ASTNode {
public:
    std::string name; ///< The function's identifier
    std::unique_ptr<ReturnStatement> body; ///< Pointer to the function's body (a return statement)

    /**
     * @brief Constructs a Function node.
     * 
     * @param id The function name (identifier).
     * @param stmt The function body (return statement).
     */
    Function(const std::string& id, std::unique_ptr<ReturnStatement> stmt)
        : name(id), body(std::move(stmt)) {}
};

/**
 * @brief Represents the root program node.
 * 
 * This corresponds to the grammar rule: ‹program› ::= ‹function›
 */
class Program : public ASTNode {
public:
    std::unique_ptr<Function> function; ///< Pointer to the function node

    /**
     * @brief Constructs a Program node.
     * 
     * @param func The top-level function of the program.
     */
    Program(std::unique_ptr<Function> func) : function(std::move(func)) {}
};

#endif // AST_HPP
