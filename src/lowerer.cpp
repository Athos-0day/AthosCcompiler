#include "lowerer.hpp"

#include <memory>
#include <string>

std::string Lowerer::newTemp() {
    return "%tmp" + std::to_string(tempCounter++);
}

std::unique_ptr<tacky::Val> Lowerer::lowerExpression(const Expression* expr) {
    if (!expr->operand) {
        // Base case: expression is a constant
        return std::make_unique<tacky::Constant>(expr->value);
    }

    // Recursive case: unary expression
    auto src = lowerExpression(expr->operand.get());
    auto tmpName = newTemp();

    tacky::UnaryOp op = (expr->op == Token::COMPLEMENT)
                            ? tacky::UnaryOp::Complement
                            : tacky::UnaryOp::Negate;

    // Emit the Unary instruction: dst = op src
    instructions.push_back(std::make_unique<tacky::Unary>(
        op,
        std::move(src),
        std::make_unique<tacky::Var>(tmpName)
    ));

    // Return the temporary variable as the result of this expression
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
