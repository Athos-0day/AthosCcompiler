/**
 * @file validate.hpp
 * @brief Semantic validation and name resolution for a simple C-like language.
 *
 * This module performs semantic analysis on an Abstract Syntax Tree (AST),
 * ensuring variables are properly declared, uniquely named, and used correctly
 * within their scope.
 */

#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include "ast.hpp"

/**
 * @typedef VarMap
 * @brief Maps user-defined variable names to internal unique names.
 */
using VarMap = std::unordered_map<std::string, std::string>;

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
 * @param varMap Map of variable names to unique internal names.
 * @return A new resolved and validated expression.
 * @throws std::runtime_error If a semantic error is found.
 */
std::unique_ptr<Expression> resolve_exp(const Expression* expr, const VarMap& varMap);

/**
 * @brief Validates and resolves a variable declaration.
 *
 * This includes:
 * - Duplicate declaration checking
 * - Unique name generation
 * - Initializer expression resolution
 *
 * @param decl Pointer to the Declaration node.
 * @param varMap Map of current scope's variables.
 * @throws std::runtime_error On duplicate declarations.
 */
void resolve_declaration(Declaration* decl, VarMap& varMap);

/**
 * @brief Validates and resolves a statement node.
 *
 * Handles:
 * - Return statements (resolves expressions)
 * - Expression statements
 * - Null statements (no-op)
 *
 * @param stmt Pointer to the Statement node.
 * @param varMap Variable scope map.
 * @throws std::runtime_error On semantic errors.
 */
void resolve_statement(Statement* stmt, VarMap& varMap);

/**
 * @brief Resolves a block item, which can be either a declaration or a statement.
 *
 * Used during function body resolution to handle mixed code blocks.
 *
 * @param item Pointer to the BlockItem node.
 * @param varMap Variable scope map.
 */
void resolve_block_item(BlockItem* item, VarMap& varMap);

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
