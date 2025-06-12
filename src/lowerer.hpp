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
 * for intermediate results.
 */
class Lowerer {
private:
    int tempCounter = 0; ///< Counter to generate fresh temporary variable names
    std::vector<std::unique_ptr<tacky::Instruction>> instructions; ///< Buffer of TACKY instructions

    /**
     * @brief Generate a new unique temporary variable name.
     * 
     * @return A string representing the temporary variable (e.g., "%tmp0").
     */
    std::string newTemp();

    /**
     * @brief Recursively lower an AST Expression node to a TACKY Val.
     * 
     * If the expression is a constant, returns a Constant IR node.
     * If it's a unary or binary operation, emits the appropriate instruction
     * and returns a Var referencing the temporary result.
     * 
     * @param expr Pointer to the Expression AST node.
     * @return A unique_ptr to the lowered TACKY Val.
     */
    std::unique_ptr<tacky::Val> lowerExpression(const Expression* expr);

    /**
     * @brief Convert an AST binary operator to its TACKY equivalent.
     * 
     * This avoids unsafe static_cast and ensures semantic correctness.
     * 
     * @param op The AST binary operator.
     * @return The corresponding TACKY binary operator.
     */
    tacky::BinaryOp toTackyBinaryOp(BinaryOpast op);

public:
    /**
     * @brief Lower an entire AST Program node into a TACKY IR Program.
     * 
     * This method lowers the function, translates all expressions into instructions,
     * and appends a Return instruction.
     * 
     * @param astProgram Pointer to the AST Program node.
     * @return A unique_ptr to the resulting TACKY IR Program.
     */
    std::unique_ptr<tacky::Program> lower(const Program* astProgram);
};

#endif // LOWERER_HPP
