/**
 * @file validate.cpp
 * @brief Implementation of semantic validation and name resolution functions with block scoping support.
 */

#include "validate.hpp"
#include <stdexcept>
#include <iostream>

bool validate_verbose = false;

void log(const std::string& msg) {
    if (validate_verbose) {
        std::cout << "[Validate] " << msg << std::endl;
    }
}

[[noreturn]] void error(const std::string& msg) {
    throw std::runtime_error("Semantic error: " + msg);
}

std::string resolve_variable_name(const std::string& name, const std::vector<VarMap>& scopes) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    error("Use of undeclared variable '" + name + "'");
}

std::unique_ptr<Expression> resolve_exp(const Expression* expr, const std::vector<VarMap>& scopes) {
    switch (expr->type) {
        case ExpressionType::CONSTANT:
            return std::make_unique<Expression>(expr->value);

        case ExpressionType::VAR: {
            std::string uniqueName = resolve_variable_name(expr->identifier, scopes);
            log("Resolved variable '" + expr->identifier + "' to '" + uniqueName + "'");
            return std::make_unique<Expression>(uniqueName);
        }

        case ExpressionType::UNARY: {
            auto sub = resolve_exp(expr->operand.get(), scopes);
            return std::make_unique<Expression>(expr->un_op, std::move(sub));
        }

        case ExpressionType::BINARY: {
            auto left = resolve_exp(expr->operand1.get(), scopes);
            auto right = resolve_exp(expr->operand2.get(), scopes);
            return std::make_unique<Expression>(expr->bin_op, std::move(left), std::move(right));
        }

        case ExpressionType::ASSIGNMENT: {
            if (!expr->exp1 || expr->exp1->type != ExpressionType::VAR) {
                error("Left-hand side of assignment must be a variable");
            }
            std::string uniqueName = resolve_variable_name(expr->exp1->identifier, scopes);
            log("Resolved assignment to '" + uniqueName + "'");
            auto lhs = std::make_unique<Expression>(uniqueName);
            auto rhs = resolve_exp(expr->exp2.get(), scopes);
            return std::make_unique<Expression>(std::move(lhs), std::move(rhs));
        }

        case ExpressionType::CONDITIONAL: {
            auto cond = resolve_exp(expr->condition.get(), scopes);
            auto thenExpr = resolve_exp(expr->trueExpr.get(), scopes);
            auto elseExpr = resolve_exp(expr->falseExpr.get(), scopes);
            return std::make_unique<Expression>(std::move(cond), std::move(thenExpr), std::move(elseExpr));
        }

        default:
            error("Unknown expression type in semantic validation");
    }
}

void resolve_declaration(Declaration* decl, std::vector<VarMap>& scopes) {
    const std::string& name = decl->name;
    VarMap& current = scopes.back();

    if (current.find(name) != current.end()) {
        error("Variable '" + name + "' is already declared in this scope");
    }

    std::string uniqueName = generateUniqueName(name);

    current[name] = uniqueName;

    log("Declared variable '" + name + "' as '" + uniqueName + "'");
    decl->name = uniqueName;
    
    if (decl->initializer) {
        decl->initializer = resolve_exp(decl->initializer.get(), scopes);
        log("Resolved initializer for '" + uniqueName + "'");
    }
}

void resolve_statement(Statement* stmt, std::vector<VarMap>& scopes, const std::string& currentLoopLabel) {
    switch (stmt->type) {
        case StatementType::RETURN:
            log("Resolving return statement");
            stmt->expression = resolve_exp(stmt->expression.get(), scopes);
            break;

        case StatementType::EXPRESSION:
            log("Resolving expression statement");
            stmt->expression = resolve_exp(stmt->expression.get(), scopes);
            break;

        case StatementType::NULL_STMT:
            log("Empty/null statement");
            break;

        case StatementType::IF:
            log("Resolving if statement");
            stmt->condition = resolve_exp(stmt->condition.get(), scopes);
            resolve_statement(stmt->thenBranch.get(), scopes, currentLoopLabel);
            if (stmt->elseBranch) {
                resolve_statement(stmt->elseBranch.get(), scopes, currentLoopLabel);
            }
            break;

        case StatementType::COMPOUND:
            log("Resolving compound statement (block)");
            resolve_block(stmt->block.get(), scopes, currentLoopLabel);
            break;

        case StatementType::WHILE:
        case StatementType::DO_WHILE:
        case StatementType::FOR: {
            std::string loopLabel = generateUniqueName("loop");
            stmt->label = loopLabel;
            log("Generated loop label: '" + loopLabel + "'");

            if (stmt->type == StatementType::WHILE || stmt->type == StatementType::DO_WHILE) {
                log("Resolving " + std::string(stmt->type == StatementType::WHILE ? "while" : "do-while") + " loop");
                stmt->condition = resolve_exp(stmt->condition.get(), scopes);
                resolve_statement(stmt->body.get(), scopes, loopLabel);
            } else { // FOR
                log("Resolving for loop");

                scopes.push_back(VarMap{});

                if (stmt->forInit) {
                    if (stmt->forInit->type == ForInitType::INIT_DECL) {
                        resolve_declaration(stmt->forInit->decl.get(), scopes);
                    } else if (stmt->forInit->type == ForInitType::INIT_EXP && stmt->forInit->expr) {
                        stmt->forInit->setExpression(resolve_exp(stmt->forInit->expr.get(), scopes));
                    }
                }

                if (stmt->condition) {
                    stmt->condition = resolve_exp(stmt->condition.get(), scopes);
                }

                if (stmt->postExpr) {
                    stmt->postExpr = resolve_exp(stmt->postExpr.get(), scopes);
                }

                resolve_statement(stmt->body.get(), scopes, loopLabel);

                scopes.pop_back();
            }
            break;
        }

        case StatementType::BREAK:
        case StatementType::CONTINUE:
            if (currentLoopLabel.empty()) {
                error("break/continue used outside of a loop");
            }
            stmt->label = currentLoopLabel;
            log("Assigned loop label '" + currentLoopLabel + "' to " + 
                (stmt->type == StatementType::BREAK ? "break" : "continue") + " statement");
            break;

        default:
            error("Unsupported statement type");
    }
}

void resolve_block_item(BlockItem* item, std::vector<VarMap>& scopes) {
    switch (item->type) {
        case BlockItemType::DECLARATION:
            resolve_declaration(item->declaration.get(), scopes);
            break;

        case BlockItemType::STATEMENT:
            resolve_statement(item->statement.get(), scopes);
            break;

        default:
            error("Invalid block item type");
    }
}

void resolve_block(Block* block, std::vector<VarMap>& scopes, const std::string& currentLoopLabel) {
    scopes.push_back(VarMap{}); // Enter new scope
    for (auto& item : block->items) {
        switch (item->type) {
            case BlockItemType::DECLARATION:
                resolve_declaration(item->declaration.get(), scopes);
                break;
            case BlockItemType::STATEMENT:
                resolve_statement(item->statement.get(), scopes, currentLoopLabel);
                break;
            default:
                error("Invalid block item type");
        }
    }
    scopes.pop_back(); // Exit scope
}

void resolve_function(Function* fn) {
    log("Resolving function '" + fn->name + "'");
    std::vector<VarMap> scopes;
    scopes.emplace_back();  // global scope for this function

    resolve_block(fn->body.get(), scopes, "");

    log("Finished resolving function '" + fn->name + "'");
}

void resolve_program(Program* program) {
    if (!program || !program->function) {
        error("Program is missing a function definition");
    }
    resolve_function(program->function.get());
}
