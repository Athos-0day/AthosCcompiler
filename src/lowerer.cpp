#include "lowerer.hpp"
#include "ast.hpp"

#include <memory>
#include <string>
#include <stdexcept>

std::string Lowerer::newTemp() {
    return "%tmp" + std::to_string(tempCounter++);
}

tacky::BinaryOp Lowerer::toTackyBinaryOp(BinaryOpast op) {
    switch (op) {
        case BinaryOpast::ADD:        return tacky::BinaryOp::ADD;
        case BinaryOpast::SUBTRACT:   return tacky::BinaryOp::SUBTRACT;
        case BinaryOpast::MULTIPLY:   return tacky::BinaryOp::MULTIPLY;
        case BinaryOpast::DIVIDE:     return tacky::BinaryOp::DIVIDE;
        case BinaryOpast::REMAINDER:  return tacky::BinaryOp::REMAINDER;
        default:
            throw std::runtime_error("Invalid BinaryOpast encountered in Lowerer::toTackyBinaryOp");
    }
}

std::unique_ptr<tacky::Val> Lowerer::lowerExpression(const Expression* expr) {
    // Case 1: Binary operation
    if (expr->operand1 && expr->operand2) {
        auto lhs = lowerExpression(expr->operand1.get());
        auto rhs = lowerExpression(expr->operand2.get());
        auto tmpName = newTemp();

        tacky::BinaryOp op = toTackyBinaryOp(expr->bin_op);

        instructions.push_back(std::make_unique<tacky::Binary>(
            op,
            std::move(lhs),
            std::move(rhs),
            std::make_unique<tacky::Var>(tmpName)
        ));

        return std::make_unique<tacky::Var>(tmpName);
    }

    // Case 2: Constant literal
    if (!expr->operand) {
        return std::make_unique<tacky::Constant>(expr->value);
    }

    // Case 3: Unary operation
    auto src = lowerExpression(expr->operand.get());
    auto tmpName = newTemp();

    tacky::UnaryOp op = (expr->un_op == UnaryOpast::COMPLEMENT)
                            ? tacky::UnaryOp::Complement
                            : tacky::UnaryOp::Negate;

    instructions.push_back(std::make_unique<tacky::Unary>(
        op,
        std::move(src),
        std::make_unique<tacky::Var>(tmpName)
    ));

    return std::make_unique<tacky::Var>(tmpName);
}

std::unique_ptr<tacky::Program> Lowerer::lower(const Program* astProgram) {
    // Create the function object with the given name
    auto func = std::make_unique<tacky::Function>(astProgram->function->name);

    // Get the return statement from the function body
    const ReturnStatement* ret = astProgram->function->body.get();

    // Lower the return expression to TACKY IR
    auto val = lowerExpression(ret->expression.get());

    // Assign all generated instructions to the function body
    func->body = std::move(instructions);

    // Append the final Return instruction with the lowered value
    func->body.push_back(std::make_unique<tacky::Return>(std::move(val)));

    // Return the full program IR containing the function
    return std::make_unique<tacky::Program>(std::move(func));
}
