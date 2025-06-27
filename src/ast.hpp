#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <memory>
#include <vector>
#include "lexer.hpp"

/**
 * @brief Enum class for expression types.
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
 * @brief Enum class for block item types.
 */
enum class BlockItemType {
    STATEMENT,
    DECLARATION
};

/**
 * @brief Enum class for statement types.
 */
enum class StatementType {
    RETURN,
    EXPRESSION,
    NULL_STMT,
    IF,
    COMPOUND,
    BREAK,
    CONTINUE,
    WHILE,
    DO_WHILE,
    FOR
};

/**
 * @brief Enum class for unary operators.
 */
enum class UnaryOpast {
    COMPLEMENT,
    NEGATE,
    NOT
};

/**
 * @brief Enum class for binary operators.
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
 * @brief Enum class for For_Init 
 */
enum class ForInitType {
    INIT_DECL,
    INIT_EXP
};

/**
 * @brief Base class for all Abstract Syntax Tree (AST) nodes.
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

    // Constant
    int value = 0;

    // Unary expression
    UnaryOpast un_op;
    std::unique_ptr<Expression> operand;

    // Binary expression
    BinaryOpast bin_op;
    std::unique_ptr<Expression> operand1;
    std::unique_ptr<Expression> operand2;

    // Variable expression
    std::string identifier;

    // Assignment expression
    std::unique_ptr<Expression> exp1;
    std::unique_ptr<Expression> exp2;

    // Conditional (ternary) expression
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> trueExpr;
    std::unique_ptr<Expression> falseExpr;

    // Constructors
    Expression(int v)
        : type(ExpressionType::CONSTANT), value(v) {}

    Expression(UnaryOpast unaryOp, std::unique_ptr<Expression> expr)
        : type(ExpressionType::UNARY), un_op(unaryOp), operand(std::move(expr)) {}

    Expression(BinaryOpast binaryOp, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : type(ExpressionType::BINARY), bin_op(binaryOp), operand1(std::move(lhs)), operand2(std::move(rhs)) {}

    Expression(std::string id)
        : type(ExpressionType::VAR), identifier(std::move(id)) {}

    Expression(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : type(ExpressionType::ASSIGNMENT), exp1(std::move(lhs)), exp2(std::move(rhs)) {}

    Expression(std::unique_ptr<Expression> cond,
               std::unique_ptr<Expression> tExpr,
               std::unique_ptr<Expression> fExpr)
        : type(ExpressionType::CONDITIONAL),
          condition(std::move(cond)),
          trueExpr(std::move(tExpr)),
          falseExpr(std::move(fExpr)) {}
};

/**
 * @brief Represents a variable declaration (optionally with initializer).
 */
class Declaration : public ASTNode {
public:
    std::string name;
    std::unique_ptr<Expression> initializer;

    Declaration(const std::string& id, std::unique_ptr<Expression> init = nullptr)
        : name(id), initializer(std::move(init)) {}
};

/** 
 * @brief Represents a For Init node. 
 */
class ForInit : public ASTNode {
public:
    ForInitType type;
    std::unique_ptr<Declaration> decl;
    std::unique_ptr<Expression> expr;

    void setExpression(std::unique_ptr<Expression> e) { expr = std::move(e); }
    
    ForInit(std::unique_ptr<Declaration> d) 
        : type(ForInitType::INIT_DECL), decl(std::move(d)) {}
    
    ForInit(std::unique_ptr<Expression> e)
        : type(ForInitType::INIT_EXP), expr(std::move(e)) {}
};

class Block;

/**
 * @brief Represents a statement node (return, expression, null, if, or compound).
 */
class Statement : public ASTNode {
public:
    StatementType type;

    // Optional label
    std::string label;

    // For return or expression statements
    std::unique_ptr<Expression> expression;

    // For if / while / etc.
    std::unique_ptr<Expression> condition;

    // For loop bodies and condition branches
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;  // used for IF
    std::unique_ptr<Statement> body;        // used for loops

    // For compound statements
    std::unique_ptr<Block> block;

    // For 'for' loops
    std::unique_ptr<ForInit> forInit;
    std::unique_ptr<Expression> postExpr;

    // Constructors
    Statement(std::unique_ptr<Expression> expr, StatementType type)
        : type(type), expression(std::move(expr)) {}

    Statement(StatementType type)
        : type(type) {}

    Statement(std::unique_ptr<Block> b)
        : type(StatementType::COMPOUND), block(std::move(b)) {}

    Statement(std::unique_ptr<Expression> cond,
              std::unique_ptr<Statement> thenStmt,
              std::unique_ptr<Statement> elseStmt = nullptr)
        : type(StatementType::IF),
          condition(std::move(cond)),
          thenBranch(std::move(thenStmt)),
          elseBranch(std::move(elseStmt)) {}

    // WHILE and DO_WHILE: while (cond) body;
    Statement(std::unique_ptr<Expression> cond,
              std::unique_ptr<Statement> bodyStmt,
              StatementType t,
              const std::string& lbl = "")
        : type(t),
          label(lbl),
          condition(std::move(cond)),
          body(std::move(bodyStmt)) {}

    // FOR: for (init; cond; post) body;
    Statement(std::unique_ptr<ForInit> init,
              std::unique_ptr<Expression> cond,
              std::unique_ptr<Expression> post,
              std::unique_ptr<Statement> bodyStmt,
              const std::string& lbl = "")
        : type(StatementType::FOR),
          label(lbl),
          forInit(std::move(init)),
          condition(std::move(cond)),
          postExpr(std::move(post)),
          body(std::move(bodyStmt)) {}

    // BREAK and CONTINUE
    static std::unique_ptr<Statement> makeBreak(const std::string& lbl = "") {
        auto stmt = std::make_unique<Statement>(StatementType::BREAK);
        stmt->label = lbl;
        return stmt;
    }

    static std::unique_ptr<Statement> makeContinue(const std::string& lbl = "") {
        auto stmt = std::make_unique<Statement>(StatementType::CONTINUE);
        stmt->label = lbl;
        return stmt;
    }
};


/**
 * @brief Represents a block item: either a statement or a declaration.
 */
class BlockItem : public ASTNode {
public:
    BlockItemType type;

    std::unique_ptr<Statement> statement;
    std::unique_ptr<Declaration> declaration;

    BlockItem(std::unique_ptr<Statement> stmt)
        : type(BlockItemType::STATEMENT), statement(std::move(stmt)) {}

    BlockItem(std::unique_ptr<Declaration> decl)
        : type(BlockItemType::DECLARATION), declaration(std::move(decl)) {}
};

/**
 * @brief Represents a compound block of code containing multiple block items.
 */
class Block : public ASTNode {
public:
    std::vector<std::unique_ptr<BlockItem>> items;

    Block(std::vector<std::unique_ptr<BlockItem>>&& items)
        : items(std::move(items)) {}
};

/**
 * @brief Represents a function definition.
 */
class Function : public ASTNode {
public:
    std::string name;
    std::unique_ptr<Block> body;

    Function(const std::string& id, std::unique_ptr<Block> b)
        : name(id), body(std::move(b)) {}
};

/**
 * @brief Represents the root program node (entry point).
 */
class Program : public ASTNode {
public:
    std::unique_ptr<Function> function;

    Program(std::unique_ptr<Function> func)
        : function(std::move(func)) {}
};

#endif // AST_HPP
