#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <memory>
#include "lexer.hpp"
#include <vector>

/**
 * @brief Enum class for expression type.
 */
enum class ExpressionType {
    CONSTANT,
    UNARY,
    BINARY,
    VAR,
    ASSIGNMENT,
    CONDITIONAL
};

/**
 * @brief Enum class for block item type.
 */
enum class BlockItemType {
    STATEMENT,
    DECLARATION
};

/**
 * @brief Enum class for statement type.
 */
enum class StatementType {
    RETURN,
    EXPRESSION,
    NULL_STMT,
    IF
};

/**
 * @brief Enum class for unary operator.
 */
enum class UnaryOpast {
    COMPLEMENT,
    NEGATE,
    NOT
};

/**
 * @brief Enum class for binary operator.
 */
enum class BinaryOpast {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    REMAINDER,
    AND,
    OR,
    EQUAL,
    NOTEQUAL,
    LESSTHAN,
    LESSEQ,
    GREATERTHAN,
    GREATEREQ
};

/**
 * @brief Base class for all nodes in the Abstract Syntax Tree (AST).
 */
class ASTNode {
public:
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

    // For var expressions
    std::string identifier;

    // For assignment expressions
    std::unique_ptr<Expression> exp1;
    std::unique_ptr<Expression> exp2;

    // For conditional expressions
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> trueExpr;
    std::unique_ptr<Expression> falseExpr;

    // Constant constructor
    Expression(int v)
        : type(ExpressionType::CONSTANT), value(v) {}

    // Unary expression constructor
    Expression(UnaryOpast unaryOp, std::unique_ptr<Expression> expr)
        : type(ExpressionType::UNARY), un_op(unaryOp), operand(std::move(expr)) {}

    // Binary expression constructor
    Expression(BinaryOpast binaryOp, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : type(ExpressionType::BINARY), bin_op(binaryOp), operand1(std::move(lhs)), operand2(std::move(rhs)) {}

    // Var constructor 
    Expression(std::string id) 
        : type(ExpressionType::VAR), identifier(id) {}

    // Assignment constructor 
    Expression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : type(ExpressionType::ASSIGNMENT), exp1(std::move(lhs)), exp2(std::move(rhs)) {}

    // Conditional (ternary) constructor: condition ? trueExpr : falseExpr
    Expression(std::unique_ptr<Expression> cond,
               std::unique_ptr<Expression> tExpr,
               std::unique_ptr<Expression> fExpr)
        : type(ExpressionType::CONDITIONAL),
          condition(std::move(cond)),
          trueExpr(std::move(tExpr)),
          falseExpr(std::move(fExpr)) {}
};

/**
 * @brief Represents a statement node, including return, expression, null, and if statements.
 */
class Statement : public ASTNode {
public:
    StatementType type;

    // For return or expression statements
    std::unique_ptr<Expression> expression;

    // For IF statement
    std::unique_ptr<Expression> condition;          ///< Condition expression
    std::unique_ptr<Statement> thenBranch;          ///< Statement to execute if condition is true
    std::unique_ptr<Statement> elseBranch;          ///< Optional else branch

    // Constructor for return or expression statements
    Statement(std::unique_ptr<Expression> expr, StatementType type)
        : type(type), expression(std::move(expr)) {}

    // Constructor for null statements
    Statement(StatementType type)
        : type(type) {}

    // Constructor for IF statement
    Statement(std::unique_ptr<Expression> cond,
              std::unique_ptr<Statement> thenStmt,
              std::unique_ptr<Statement> elseStmt = nullptr)
        : type(StatementType::IF),
          condition(std::move(cond)),
          thenBranch(std::move(thenStmt)),
          elseBranch(std::move(elseStmt)) {}
};

/**
 * @brief Represents a variable declaration.
 */
class Declaration : public ASTNode {
public:
    std::string name;
    std::unique_ptr<Expression> initializer; // optional

    // Constructor for declaration with optional initializer
    Declaration(const std::string& id, std::unique_ptr<Expression> init = nullptr)
        : name(id), initializer(std::move(init)) {}
};

/**
 * @brief Represents either a statement or a declaration in a block.
 */
class BlockItem : public ASTNode {
public:
    BlockItemType type;

    std::unique_ptr<Statement> statement;
    std::unique_ptr<Declaration> declaration;

    // Constructor for statement
    BlockItem(std::unique_ptr<Statement> stmt)
        : type(BlockItemType::STATEMENT), statement(std::move(stmt)) {}

    // Constructor for declaration
    BlockItem(std::unique_ptr<Declaration> decl)
        : type(BlockItemType::DECLARATION), declaration(std::move(decl)) {}
};


/**
 * @brief Represents a function definition with a list of block items.
 */
class Function : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<BlockItem>> body;

    Function(const std::string& id, std::vector<std::unique_ptr<BlockItem>>&& items)
        : name(id), body(std::move(items)) {}
};

/**
 * @brief Represents the root program node.
 */
class Program : public ASTNode {
public:
    std::unique_ptr<Function> function;

    Program(std::unique_ptr<Function> func) : function(std::move(func)) {}
};

#endif // AST_HPP
