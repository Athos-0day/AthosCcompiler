#include "ast.hpp"
#include <iostream>

/**
 * @brief Print an expression.
 */
void printExpression(const Expression* expr) {
    if (!expr) return;

    switch (expr->type) {
        case ExpressionType::CONSTANT:
            std::cout << expr->value;
            break;

        case ExpressionType::VAR:
            std::cout << expr->identifier;
            break;

        case ExpressionType::UNARY:
            std::cout << "(";
            switch (expr->un_op) {
                case UnaryOpast::COMPLEMENT: std::cout << "~"; break;
                case UnaryOpast::NEGATE:     std::cout << "-"; break;
                case UnaryOpast::NOT:        std::cout << "!"; break;
            }
            printExpression(expr->operand.get());
            std::cout << ")";
            break;

        case ExpressionType::BINARY:
            std::cout << "(";
            printExpression(expr->operand1.get());

            switch (expr->bin_op) {
                case BinaryOpast::ADD: std::cout << " + "; break;
                case BinaryOpast::SUBTRACT: std::cout << " - "; break;
                case BinaryOpast::MULTIPLY: std::cout << " * "; break;
                case BinaryOpast::DIVIDE: std::cout << " / "; break;
                case BinaryOpast::REMAINDER: std::cout << " % "; break;
                case BinaryOpast::AND: std::cout << " && "; break;
                case BinaryOpast::OR: std::cout << " || "; break;
                case BinaryOpast::EQUAL: std::cout << " == "; break;
                case BinaryOpast::NOTEQUAL: std::cout << " != "; break;
                case BinaryOpast::LESSTHAN: std::cout << " < "; break;
                case BinaryOpast::LESSEQ: std::cout << " <= "; break;
                case BinaryOpast::GREATERTHAN: std::cout << " > "; break;
                case BinaryOpast::GREATEREQ: std::cout << " >= "; break;
            }

            printExpression(expr->operand2.get());
            std::cout << ")";
            break;

        case ExpressionType::ASSIGNMENT:
            std::cout << "(";
            printExpression(expr->exp1.get());
            std::cout << " = ";
            printExpression(expr->exp2.get());
            std::cout << ")";
            break;
    }
}

/**
 * @brief Print a statement.
 */
void printStatement(const Statement* stmt) {
    if (!stmt) return;

    switch (stmt->type) {
        case StatementType::RETURN:
            std::cout << "return ";
            printExpression(stmt->expression.get());
            std::cout << ";" << std::endl;
            break;

        case StatementType::EXPRESSION:
            printExpression(stmt->expression.get());
            std::cout << ";" << std::endl;
            break;

        case StatementType::NULL_STMT:
            std::cout << ";" << std::endl;
            break;
    }
}

/**
 * @brief Print a declaration.
 */
void printDeclaration(const Declaration* decl) {
    if (!decl) return;

    std::cout << "int " << decl->name;
    if (decl->initializer) {
        std::cout << " = ";
        printExpression(decl->initializer.get());
    }
    std::cout << ";" << std::endl;
}

/**
 * @brief Print a block item.
 */
void printBlockItem(const BlockItem* item) {
    if (!item) return;

    switch (item->type) {
        case BlockItemType::STATEMENT:
            printStatement(item->statement.get());
            break;

        case BlockItemType::DECLARATION:
            printDeclaration(item->declaration.get());
            break;
    }
}

/**
 * @brief Print a function.
 */
void printFunction(const Function* func) {
    std::cout << "int " << func->name << "(void) {" << std::endl;
    for (const auto& item : func->body) {
        printBlockItem(item.get());
    }
    std::cout << "}" << std::endl;
}

/**
 * @brief Print a program.
 */
void printProgram(const Program* prog) {
    printFunction(prog->function.get());
}

