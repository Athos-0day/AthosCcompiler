/**
 * @file validate.hpp
 * @brief Semantic validation and name resolution for a simple C-like language.
 *
 * This module performs semantic analysis on an Abstract Syntax Tree (AST),
 * ensuring variables are properly declared, uniquely named, and used correctly
 * within their scope and supports nested block scopes.
 */

#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "ast.hpp"

/**
 * @typedef VarMap
 * @brief Maps user-defined variable names to internal unique names within a single scope.
 */
using VarMap = std::unordered_map<std::string, std::string>;

/**
 * @typedef VarMapStack
 * @brief Stack of VarMaps representing nested variable scopes.
 *
 * The back of the vector is the current (innermost) scope.
 */
using VarMapStack = std::vector<VarMap>;

/**
 * @brief Global flag to enable verbose logging during validation.
 */
extern bool validate_verbose;

/**
 * @brief Generates a unique internal name for a given variable.
 *
 * Used to avoid naming collisions during code generation or further analysis.
 *
 * @param baseName The original variable name as defined in the source code.
 * @return A unique internal name.
 */
inline std::string generateUniqueName(const std::string& baseName) {
    static int counter = 0;
    return baseName + "_" + std::to_string(counter++);
}

/**
 * @brief Resolves and validates an expression node.
 *
 * This function replaces variable references with their unique names,
 * validates expressions recursively, and checks for correct usage
 * of assignment and operator types.
 *
 * @param expr The expression node to resolve.
 * @param varStack Stack of variable scopes, from outermost to innermost.
 * @return A new resolved and validated expression.
 * @throws std::runtime_error If a semantic error is found.
 */
std::unique_ptr<Expression> resolve_exp(const Expression* expr, const VarMapStack& varStack);

/**
 * @brief Validates and resolves a variable declaration.
 *
 * This includes:
 * - Duplicate declaration checking in the current (innermost) scope
 * - Unique name generation
 * - Initializer expression resolution
 *
 * @param decl Pointer to the Declaration node.
 * @param varStack Stack of variable scopes.
 * @throws std::runtime_error On duplicate declarations.
 */
void resolve_declaration(Declaration* decl, VarMapStack& varStack);

/**
 * @brief Validates and resolves a statement node.
 *
 * Handles:
 * - Return statements (resolves expressions)
 * - Expression statements
 * - Null statements (no-op)
 * - If statements (including nested branches)
 * - Compound statements (blocks), which introduce new scopes
 *
 * @param stmt Pointer to the Statement node.
 * @param varStack Stack of variable scopes.
 * @throws std::runtime_error On semantic errors.
 */
void resolve_statement(Statement* stmt, VarMapStack& varStack);

/**
 * @brief Resolves all semantics within a compound block statement.
 *
 * This function manages a new variable scope, resolves declarations
 * and statements inside the block, and properly handles nested scopes.
 *
 * @param block Pointer to the Block node representing the compound statement.
 * @param scopes Reference to the stack of variable scopes.
 */
void resolve_block(Block* block, std::vector<VarMap>& scopes);

/**
 * @brief Resolves a block item, which can be either a declaration or a statement.
 *
 * Used during function body resolution to handle mixed code blocks.
 *
 * @param item Pointer to the BlockItem node.
 * @param varStack Stack of variable scopes.
 */
void resolve_block_item(BlockItem* item, VarMapStack& varStack);

/**
 * @brief Resolves all semantics within a function body.
 *
 * Iterates over all block items, validates declarations and statements,
 * and maintains proper scoping rules.
 *
 * @param fn Pointer to the Function node to validate.
 */
void resolve_function(Function* fn);

/**
 * @brief Resolves and validates the entire program AST.
 *
 * This should be called on the root Program node after parsing.
 *
 * @param program Pointer to the root Program node.
 * @throws std::runtime_error On any semantic validation failure.
 */
void resolve_program(Program* program);

#endif // VALIDATE_HPP
