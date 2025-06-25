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

        // Standard binary op
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

    // Constant
    if (!expr->operand && expr->type == ExpressionType::CONSTANT) {
        return std::make_unique<tacky::Constant>(expr->value);
    }

    // Variable
    if (expr->type == ExpressionType::VAR) {
        return std::make_unique<tacky::Var>(expr->identifier);
    }

    // Assignment (only var = expr form supported)
    if (expr->type == ExpressionType::ASSIGNMENT && expr->exp1->type == ExpressionType::VAR) {
        auto rhs = lowerExpression(expr->exp2.get());
        std::string lhsName = expr->exp1->identifier;

        instructions.push_back(std::make_unique<tacky::Copy>(
            std::move(rhs),
            std::make_unique<tacky::Var>(lhsName)
        ));

        return std::make_unique<tacky::Var>(lhsName);
    }

    if (expr->type == ExpressionType::CONDITIONAL) {
        auto dst = newTemp();  // temporary variable to hold the result

        std::string elseLabel = newLabel("cond_else");
        std::string endLabel = newLabel("cond_end");

        // Lower the condition expression
        auto condVal = lowerExpression(expr->condition.get());

        // If condition is false, jump to else
        instructions.push_back(std::make_unique<tacky::JumpIfZero>(std::move(condVal), elseLabel));

        // True branch: evaluate and copy to dst
        auto trueVal = lowerExpression(expr->trueExpr.get());
        instructions.push_back(std::make_unique<tacky::Copy>(std::move(trueVal), std::make_unique<tacky::Var>(dst)));

        instructions.push_back(std::make_unique<tacky::Jump>(endLabel));

        // Else label
        instructions.push_back(std::make_unique<tacky::Label>(elseLabel));

        // False branch: evaluate and copy to dst
        auto falseVal = lowerExpression(expr->falseExpr.get());
        instructions.push_back(std::make_unique<tacky::Copy>(std::move(falseVal), std::make_unique<tacky::Var>(dst)));

        // End label
        instructions.push_back(std::make_unique<tacky::Label>(endLabel));

        return std::make_unique<tacky::Var>(dst);
    }


    // Unary
    if (expr->operand) {
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

    throw std::runtime_error("Unhandled expression type");
}

void Lowerer::lowerStatement(const Statement* stmt) {
    switch (stmt->type) {
        case StatementType::RETURN: {
            auto val = lowerExpression(stmt->expression.get());
            instructions.push_back(std::make_unique<tacky::Return>(std::move(val)));
            break;
        }
        case StatementType::EXPRESSION: {
            lowerExpression(stmt->expression.get());
            break;
        }
        case StatementType::IF: {
            auto condVal = lowerExpression(stmt->condition.get());

            std::string elseLabel = newLabel("else");
            std::string endLabel = newLabel("endif");

            if (stmt->elseBranch) {
                instructions.push_back(std::make_unique<tacky::JumpIfZero>(std::move(condVal), elseLabel));

                lowerStatement(stmt->thenBranch.get());

                instructions.push_back(std::make_unique<tacky::Jump>(endLabel));

                instructions.push_back(std::make_unique<tacky::Label>(elseLabel));

                lowerStatement(stmt->elseBranch.get());
                instructions.push_back(std::make_unique<tacky::Label>(endLabel));
            } else {
                instructions.push_back(std::make_unique<tacky::JumpIfZero>(std::move(condVal), endLabel));

                lowerStatement(stmt->thenBranch.get());

                instructions.push_back(std::make_unique<tacky::Jump>(endLabel));

                instructions.push_back(std::make_unique<tacky::Label>(endLabel));
            }

            break;
        }
        case StatementType::COMPOUND: {
            lowerBlock(stmt->block.get());
        }
        case StatementType::NULL_STMT:
            break;
    }
}

void Lowerer::lowerBlock(const Block* block) {
    if (!block) return; // Safety check for null pointer

    // Iterate over each item in the block and lower it
    for (const auto& item : block->items) {
        lowerBlockItem(item.get());
    }
}

void Lowerer::lowerBlockItem(const BlockItem* item) {
    if (item->type == BlockItemType::STATEMENT) {
        lowerStatement(item->statement.get());
    } else if (item->type == BlockItemType::DECLARATION) {
        std::string varName = item->declaration->name;

        if (item->declaration->initializer) {
            auto val = lowerExpression(item->declaration->initializer.get());
            instructions.push_back(std::make_unique<tacky::Copy>(
                std::move(val),
                std::make_unique<tacky::Var>(varName)
            ));
        }
    }
}

void Lowerer::lowerFunction(const Function* fn) {
    lowerBlock(fn->body.get());
}

std::unique_ptr<tacky::Program> Lowerer::lower(const Program* astProgram) {
    auto func = std::make_unique<tacky::Function>(astProgram->function->name);

    lowerFunction(astProgram->function.get());

    func->body = std::move(instructions);

    return std::make_unique<tacky::Program>(std::move(func));
}
