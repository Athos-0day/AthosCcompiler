#ifndef LOWERER_HPP
#define LOWERER_HPP

#include "ast.hpp"
#include "tacky.hpp"

#include <memory>
#include <vector>
#include <string>

/**
 * @brief The Lowerer class translates an AST (Abstract Syntax Tree)
 *        into the TACKY intermediate representation (IR).
 * 
 * It performs a post-order traversal of the AST expressions and
 * generates a flat list of TACKY instructions using temporary variables
 * and labels for control flow constructs.
 */
class Lowerer {
private:
    int tempCounter = 0; ///< Counter to generate fresh temporary variable names
    int labelCounter = 0; ///< Counter to generate fresh labels
    std::vector<std::unique_ptr<tacky::Instruction>> instructions; ///< Buffer of TACKY instructions

    /**
     * @brief Generate a new unique temporary variable name.
     * 
     * @return A string representing the temporary variable (e.g., "%tmp0").
     */
    std::string newTemp();

    /**
     * @brief Generate a new unique label name based on a base string.
     * 
     * @param base The base name for the label (e.g., "true", "false", "end").
     * @return A unique label string (e.g., "false_1").
     */
    std::string newLabel(const std::string& base);

    /**
     * @brief Convert an AST binary operator to its TACKY equivalent.
     * 
     * Ensures semantic correctness in the translation process.
     * 
     * @param op The AST binary operator.
     * @return The corresponding TACKY binary operator.
     */
    tacky::BinaryOp toTackyBinaryOp(BinaryOpast op);

    /**
     * @brief Recursively lower an AST Expression node to a TACKY Val.
     * 
     * Handles constants, variables, unary/binary expressions,
     * logical AND/OR with short-circuiting, and assignments.
     * 
     * @param expr Pointer to the AST Expression node.
     * @return A unique_ptr to the lowered TACKY Val.
     */
    std::unique_ptr<tacky::Val> lowerExpression(const Expression* expr);

    /**
     * @brief Lower a single AST Statement into TACKY instructions.
     * 
     * Supported statement types: return, expression statements, null.
     * 
     * @param stmt Pointer to the AST Statement node.
     */
    void lowerStatement(const Statement* stmt);

    /**
     * @brief Lower a single BlockItem (either a declaration or statement).
     * 
     * For declarations, initializes the variable if an initializer is provided.
     * 
     * @param item Pointer to the BlockItem node.
     */
    void lowerBlockItem(const BlockItem* item);

    /**
     * @brief Lower the body of a Function node by lowering all its BlockItems.
     * 
     * @param fn Pointer to the Function AST node.
     */
    void lowerFunction(const Function* fn);

public:
    /**
     * @brief Lower an entire AST Program node into a TACKY IR Program.
     * 
     * @param astProgram Pointer to the AST Program node.
     * @return A unique_ptr to the resulting TACKY IR Program.
     */
    std::unique_ptr<tacky::Program> lower(const Program* astProgram);
};

#endif // LOWERER_HPP
