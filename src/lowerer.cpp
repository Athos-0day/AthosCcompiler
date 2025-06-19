#include "lowerer.hpp"
#include "ast.hpp"

#include <memory>
#include <string>
#include <stdexcept>

std::string Lowerer::newTemp() {
    return "%tmp" + std::to_string(tempCounter++);
}

std::string Lowerer::newLabel(const std::string& base) {
    return base + "_" + std::to_string(labelCounter++);
}

tacky::BinaryOp Lowerer::toTackyBinaryOp(BinaryOpast op) {
    switch (op) {
        case BinaryOpast::ADD:         return tacky::BinaryOp::ADD;
        case BinaryOpast::SUBTRACT:    return tacky::BinaryOp::SUBTRACT;
        case BinaryOpast::MULTIPLY:    return tacky::BinaryOp::MULTIPLY;
        case BinaryOpast::DIVIDE:      return tacky::BinaryOp::DIVIDE;
        case BinaryOpast::REMAINDER:   return tacky::BinaryOp::REMAINDER;
        case BinaryOpast::EQUAL:       return tacky::BinaryOp::EQUAL;
        case BinaryOpast::NOTEQUAL:    return tacky::BinaryOp::NOTEQUAL;
        case BinaryOpast::LESSTHAN:    return tacky::BinaryOp::LESSTHAN;
        case BinaryOpast::LESSEQ:      return tacky::BinaryOp::LESSEQ;
        case BinaryOpast::GREATERTHAN: return tacky::BinaryOp::GREATERTHAN;
        case BinaryOpast::GREATEREQ:   return tacky::BinaryOp::GREATEREQ;
        default:
            throw std::runtime_error("Invalid BinaryOpast in toTackyBinaryOp");
    }
}

std::unique_ptr<tacky::Val> Lowerer::lowerExpression(const Expression* expr) {
    // Binary operation
    if (expr->operand1 && expr->operand2) {
        // Special case for short-circuit AND (&&)
        if (expr->bin_op == BinaryOpast::AND) {
            auto v1 = lowerExpression(expr->operand1.get());
            std::string result = newTemp();
            std::string falseLabel = newLabel("false");
            std::string endLabel = newLabel("end");

            instructions.push_back(std::make_unique<tacky::JumpIfZero>(
                std::move(v1), falseLabel
            ));

            auto v2 = lowerExpression(expr->operand2.get());
            instructions.push_back(std::make_unique<tacky::JumpIfZero>(
                std::move(v2), falseLabel
            ));

            instructions.push_back(std::make_unique<tacky::Copy>(
                std::make_unique<tacky::Constant>(1),
                std::make_unique<tacky::Var>(result)
            ));
            instructions.push_back(std::make_unique<tacky::Jump>(endLabel));

            instructions.push_back(std::make_unique<tacky::Label>(falseLabel));
            instructions.push_back(std::make_unique<tacky::Copy>(
                std::make_unique<tacky::Constant>(0),
                std::make_unique<tacky::Var>(result)
            ));

            instructions.push_back(std::make_unique<tacky::Label>(endLabel));

            return std::make_unique<tacky::Var>(result);
        }

        // Special case for short-circuit OR (||)
        if (expr->bin_op == BinaryOpast::OR) {
            auto v1 = lowerExpression(expr->operand1.get());
            std::string result = newTemp();
            std::string trueLabel = newLabel("true");
            std::string endLabel = newLabel("end");

            instructions.push_back(std::make_unique<tacky::JumpIfNotZero>(
                std::move(v1), trueLabel
            ));

            auto v2 = lowerExpression(expr->operand2.get());
            instructions.push_back(std::make_unique<tacky::JumpIfNotZero>(
                std::move(v2), trueLabel
            ));

            instructions.push_back(std::make_unique<tacky::Copy>(
                std::make_unique<tacky::Constant>(0),
                std::make_unique<tacky::Var>(result)
            ));
            instructions.push_back(std::make_unique<tacky::Jump>(endLabel));

            instructions.push_back(std::make_unique<tacky::Label>(trueLabel));
            instructions.push_back(std::make_unique<tacky::Copy>(
                std::make_unique<tacky::Constant>(1),
                std::make_unique<tacky::Var>(result)
            ));

            instructions.push_back(std::make_unique<tacky::Label>(endLabel));

            return std::make_unique<tacky::Var>(result);
        }

        // Standard binary operation
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

    // Constant literal
    if (!expr->operand) {
        return std::make_unique<tacky::Constant>(expr->value);
    }

    // Unary operation
    auto src = lowerExpression(expr->operand.get());
    auto tmpName = newTemp();

    tacky::UnaryOp op;
    switch (expr->un_op) {
        case UnaryOpast::COMPLEMENT: op = tacky::UnaryOp::Complement; break;
        case UnaryOpast::NEGATE:     op = tacky::UnaryOp::Negate;     break;
        case UnaryOpast::NOT:        op = tacky::UnaryOp::Not;        break;
        default:
            throw std::runtime_error("Unknown UnaryOpast in lowerExpression");
    }

    instructions.push_back(std::make_unique<tacky::Unary>(
        op,
        std::move(src),
        std::make_unique<tacky::Var>(tmpName)
    ));

    return std::make_unique<tacky::Var>(tmpName);
}

std::unique_ptr<tacky::Program> Lowerer::lower(const Program* astProgram) {
    auto func = std::make_unique<tacky::Function>(astProgram->function->name);
    const ReturnStatement* ret = astProgram->function->body.get();

    auto val = lowerExpression(ret->expression.get());

    func->body = std::move(instructions);
    func->body.push_back(std::make_unique<tacky::Return>(std::move(val)));

    return std::make_unique<tacky::Program>(std::move(func));
}
