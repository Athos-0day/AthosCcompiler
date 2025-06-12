#include "tacky.hpp"
#include <sstream>
#include <iostream>

namespace tacky {

/**
 * @brief Converts a unary operator enum to its string representation.
 * @param op The unary operator.
 * @return The string name of the operator.
 */
std::string toString(UnaryOp op) {
    switch (op) {
        case UnaryOp::Complement: return "Complement";
        case UnaryOp::Negate: return "Negate";
        default: return "UnknownUnaryOp";
    }
}

/**
 * @brief Converts a binary operator enum to its string representation.
 * @param op The binary operator
 * @return The string name of the operator
 */
std::string toString(BinaryOp op) {
    switch (op) {
        case BinaryOp::ADD: return "Add";
        case BinaryOp::SUBTRACT: return "Subtract";
        case BinaryOp::MULTIPLY: return "Multiply";
        case BinaryOp::DIVIDE: return "Divide";
        case BinaryOp::REMAINDER: return "Remainder";
        default: return "UnknownBinaryOp";
    }
}

// ======== toString methods for Val types ========

std::string Constant::toString() const {
    return "Constant(" + std::to_string(value) + ")";
}

std::string Var::toString() const {
    return "Var(" + name + ")";
}

// ======== toString methods for Instruction types ========

std::string Return::toString() const {
    return "Return(" + value->toString() + ")";
}

std::string Unary::toString() const {
    std::ostringstream oss;
    oss << "Unary(" << tacky::toString(op) << ", "
        << src->toString() << ", "
        << dst->toString() << ")";
    return oss.str();
}

std::string Binary::toString() const {
    std::ostringstream oss;
    oss << "Binary(" << tacky::toString(op) << ", "
        << src1->toString() << ", "
        << src2->toString() << ", "
        << dst->toString() << ")";
    return oss.str(); 
}

// ======== toString for Function ========

std::string Function::toString() const {
    std::ostringstream oss;
    oss << "Function(" << name << ") {\n";
    for (const auto& instr : body) {
        oss << "  " << instr->toString() << "\n";
    }
    oss << "}";
    return oss.str();
}

// ======== toString for Program ========

std::string Program::toString() const {
    std::ostringstream oss;
    oss << "Program:\n";
    oss << function->toString();
    return oss.str();
}

// ======== Optional debug print functions (stdout only) ========

void printVal(const Val* val) {
    std::cout << val->toString();
}

void printInstruction(const Instruction* instr) {
    std::cout << instr->toString() << "\n";
}

void printFunction(const Function* func) {
    std::cout << func->toString() << "\n";
}

void printProgram(const Program* prog) {
    std::cout << prog->toString() << "\n";
}

} // namespace tacky
