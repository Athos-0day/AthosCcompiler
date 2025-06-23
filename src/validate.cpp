/**
 * @file validate.cpp
 * @brief Implementation of semantic validation and name resolution functions.
 */

#include "validate.hpp"
#include <stdexcept>
#include <iostream>

bool validate_verbose = false;

/**
 * @brief Log semantic validation info if verbose is enabled.
 */
void log(const std::string& msg) {
    if (validate_verbose) {
        std::cout << "[Validate] " << msg << std::endl;
    }
}

/**
 * @brief Throw semantic error with standardized prefix.
 */
[[noreturn]] void error(const std::string& msg) {
    throw std::runtime_error("Semantic error: " + msg);
}

/**
 * @brief Resolve an expression recursively and replace variable names with unique ones.
 */
std::unique_ptr<Expression> resolve_exp(const Expression* expr, const VarMap& varMap) {
    switch (expr->type) {
        case ExpressionType::CONSTANT:
            return std::make_unique<Expression>(expr->value);

        case ExpressionType::VAR: {
            auto it = varMap.find(expr->identifier);
            if (it == varMap.end()) {
                error("Use of undeclared variable '" + expr->identifier + "'");
            }
            log("Resolved variable '" + expr->identifier + "' to '" + it->second + "'");
            return std::make_unique<Expression>(it->second);
        }

        case ExpressionType::UNARY: {
            auto sub = resolve_exp(expr->operand.get(), varMap);
            return std::make_unique<Expression>(expr->un_op, std::move(sub));
        }

        case ExpressionType::BINARY: {
            auto left = resolve_exp(expr->operand1.get(), varMap);
            auto right = resolve_exp(expr->operand2.get(), varMap);
            return std::make_unique<Expression>(expr->bin_op, std::move(left), std::move(right));
        }

        case ExpressionType::ASSIGNMENT: {
            if (!expr->exp1 || expr->exp1->type != ExpressionType::VAR) {
                error("Left-hand side of assignment must be a variable");
            }

            auto it = varMap.find(expr->exp1->identifier);
            if (it == varMap.end()) {
                error("Assignment to undeclared variable '" + expr->exp1->identifier + "'");
            }

            log("Resolved assignment to '" + it->second + "'");
            auto lhs = std::make_unique<Expression>(it->second);
            auto rhs = resolve_exp(expr->exp2.get(), varMap);
            return std::make_unique<Expression>(std::move(lhs), std::move(rhs));
        }

        default:
            error("Unknown expression type in semantic validation");
    }
}

/**
 * @brief Resolve a declaration: generate unique name and resolve initializer.
 */
void resolve_declaration(Declaration* decl, VarMap& varMap) {
    const std::string& name = decl->name;

    if (varMap.find(name) != varMap.end()) {
        error("Variable '" + name + "' is already declared");
    }

    std::string uniqueName = generateUniqueName(name);
    varMap[name] = uniqueName;

    log("Declared variable '" + name + "' as '" + uniqueName + "'");

    if (decl->initializer) {
        decl->initializer = resolve_exp(decl->initializer.get(), varMap);
        log("Resolved initializer for '" + uniqueName + "'");
    }
}

/**
 * @brief Resolve a statement: currently supports return and expression statements.
 */
void resolve_statement(Statement* stmt, VarMap& varMap) {
    switch (stmt->type) {
        case StatementType::RETURN:
            log("Resolving return statement");
            stmt->expression = resolve_exp(stmt->expression.get(), varMap);
            break;

        case StatementType::EXPRESSION:
            log("Resolving expression statement");
            stmt->expression = resolve_exp(stmt->expression.get(), varMap);
            break;

        case StatementType::NULL_STMT:
            log("Empty/null statement");
            break;

        default:
            error("Unsupported statement type");
    }
}

/**
 * @brief Resolve a block item (either a declaration or a statement).
 */
void resolve_block_item(BlockItem* item, VarMap& varMap) {
    switch (item->type) {
        case BlockItemType::DECLARATION:
            log("Resolving declaration in block");
            resolve_declaration(item->declaration.get(), varMap);
            break;

        case BlockItemType::STATEMENT:
            log("Resolving statement in block");
            resolve_statement(item->statement.get(), varMap);
            break;

        default:
            error("Invalid block item type");
    }
}

/**
 * @brief Resolve an entire function body.
 */
void resolve_function(Function* fn) {
    log("Resolving function '" + fn->name + "'");
    VarMap varMap;

    for (auto& item : fn->body) {
        resolve_block_item(item.get(), varMap);
    }

    log("Finished resolving function '" + fn->name + "'");
}

/**
 * @brief Resolve the root program.
 */
void resolve_program(Program* program) {
    if (!program || !program->function) {
        error("Program is missing a function definition");
    }

    resolve_function(program->function.get());
}
