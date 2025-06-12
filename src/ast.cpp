#include <string>
#include <memory>
#include "lexer.hpp"

/**
 * @brief Enum class for expression type.
 */
enum class ExpressionType {
    CONSTANT,
    UNARY,
    BINARY
};

/**
 * @brief Enum class for unary operator.
 */
enum class UnaryOpast {
    COMPLEMENT,
    NEGATE
};

/**
 * @brief Enum class for binary operator.
 */
enum class BinaryOpast {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    REMAINDER
};

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
 */
class Expression : public ASTNode {
public:
    ExpressionType type;

    // For constants
    int value = 0;

    // For unary expressions
    UnaryOpast un_op;
    std::unique_ptr<Expression> operand;

    // For binary expressions
    BinaryOpast bin_op;
    std::unique_ptr<Expression> operand1;
    std::unique_ptr<Expression> operand2;

    // Constant constructor
    Expression(int v)
        : type(ExpressionType::CONSTANT), value(v) {}

    // Unary expression constructor
    Expression(UnaryOpast unaryOp, std::unique_ptr<Expression> expr)
        : type(ExpressionType::UNARY), un_op(unaryOp), operand(std::move(expr)) {}

    // Binary expression constructor
    Expression(BinaryOpast binaryOp, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : type(ExpressionType::BINARY), bin_op(binaryOp), operand1(std::move(lhs)), operand2(std::move(rhs)) {}
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
