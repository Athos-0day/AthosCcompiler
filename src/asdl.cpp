#include "asdl.hpp"
#include <fstream>
#include <stdexcept>
#include <cctype>  // for std::tolower

// --- Idiv ---
Idiv::Idiv(std::unique_ptr<Operand> d) : dst(std::move(d)) {}

// --- Binary ---
Binary::Binary(BinaryOperator op, std::unique_ptr<Operand> s, std::unique_ptr<Operand> d)
    : binary_operator(op), src(std::move(s)), dst(std::move(d)) {}


// --- Imm ---

std::string Imm::toString() const {
    return "Imm(" + std::to_string(value) + ")";
}

std::string Imm::toASM() const {
    return "$" + std::to_string(value);
}

// --- Register ---

std::string Register::toString() const {
    return "Register(" + regToString(reg) + ")";
}

std::string Register::toASM() const {
    switch (reg) {
        case Reg::AX: return "%eax";
        case Reg::DX: return "%edx";
        case Reg::R10: return "%r10d";
        case Reg::R11: return "%r11d";
        default: return "%UNKNOWN_REG";
    }
}

// --- Pseudo ---

std::string Pseudo::toString() const {
    return "Pseudo(" + identifier + ")";
}

std::string Pseudo::toASM() const {
    return identifier;
}

// --- Stack ---

std::string Stack::toString() const {
    return "Stack(" + std::to_string(value) + ")";
}

std::string Stack::toASM() const {
    return std::to_string(value) + "(%rbp)";
}

// --- Mov ---

Mov::Mov(std::unique_ptr<Operand> s, std::unique_ptr<Operand> d)
    : src(std::move(s)), dst(std::move(d)) {}

std::string Mov::toString() const {
    return "Mov(src=" + src->toString() + ", dst=" + dst->toString() + ")";
}

std::string Mov::toASM() const {
    return "movl " + src->toASM() + ", " + dst->toASM();
}

// --- Unary ---

Unary::Unary(UnaryOperator u, std::unique_ptr<Operand> d)
    : unary_operator(u), dst(std::move(d)) {}

std::string Unary::toString() const {
    std::string opStr = (unary_operator == UnaryOperator::NEG) ? "NEG" : "NOT";
    return "Unary(" + opStr + ", dst=" + dst->toString() + ")";
}

std::string Unary::toASM() const {
    switch (unary_operator) {
        case UnaryOperator::NEG:
            return "negl " + dst->toASM();
        case UnaryOperator::NOT:
            return "notl " + dst->toASM();
        default:
            return "UNKNOWN_UNARY_OP";
    }
}

// --- Cdq ---
std::string Cdq::toString() const {
    return "Cdq";
}

std::string Cdq::toASM() const {
    return "cdq";
}

// --- Idiv ---
std::string Idiv::toString() const {
    return "Idiv(dst= " + dst->toString() + ")";
}

std::string Idiv::toASM() const {
    return "idivl " + dst->toASM();
}

// --- Binary ---
std::string Binary::toString() const {
    std::string opStr;
    if (binary_operator == BinaryOperator::ADD) {
        opStr = "ADD";
    } else if (binary_operator == BinaryOperator::SUB) {
        opStr = "SUB";
    } else {
        opStr = "MULT";
    }

    return "Binary(" + opStr + ", " + src->toString() + ", " + dst->toString() + ")";
}

std::string Binary::toASM() const {
    switch (binary_operator) {
        case BinaryOperator::ADD:
            return "addl " + src->toASM() + ", " + dst->toASM();
        case BinaryOperator::SUB:
            return "subl " + src->toASM() + ", " + dst->toASM();
        case BinaryOperator::MULT:
            return "imull " + src->toASM() + ", " + dst->toASM();
        default:
            return "UNKNOWN_BINARY_OP";
    }
}

// --- AllocateStack ---

AllocateStack::AllocateStack(int n) : value(n) {}

std::string AllocateStack::toString() const {
    return "AllocateStack(" + std::to_string(value) + ")";
}

std::string AllocateStack::toASM() const {
    return "subq $" + std::to_string(value) + ", %rsp";
}

// --- Ret ---

std::string Ret::toString() const {
    return "Ret";
}

std::string Ret::toASM() const {
    std::string codeRet = "movq %rbp, %rsp\n";
    codeRet += std::string("  ") + "popq %rbp\n";
    codeRet += std::string("  ") + "ret";
    return codeRet;
}

// --- FunctionDefinition ---

FunctionDefinition::FunctionDefinition(std::string n, std::vector<std::unique_ptr<Instruction>> ins)
    : name(std::move(n)), instructions(std::move(ins)) {}

const std::string& FunctionDefinition::getName() const {
    return name;
}

const std::vector<std::unique_ptr<Instruction>>& FunctionDefinition::getInstructions() const {
    return instructions;
}

std::string FunctionDefinition::toString() const {
    std::string result = "FunctionDefinition(name=" + name + ", instructions=[\n";
    for (const auto& instr : instructions) {
        result += "  " + instr->toString() + "\n";
    }
    result += "])";
    return result;
}

std::string FunctionDefinition::toASM() const {
    std::string asmCode = ".globl _" + name + "\n_" + name + ":\n";
    asmCode += std::string("  ") + "pushq %rbp\n";
    asmCode += std::string("  ") + "movq %rsp, %rbp\n";
    for (const auto& instr : instructions) {
        asmCode += "  " + instr->toASM() + "\n";
    }
    return asmCode;
}

// --- ASDLProgram ---

ASDLProgram::ASDLProgram(std::unique_ptr<FunctionDefinition> funcDef)
    : functionDefinition(std::move(funcDef)) {}

std::string ASDLProgram::toString() const {
    return "ASDLProgram(" + functionDefinition->toString() + ")";
}

std::string ASDLProgram::toASM() const {
    return functionDefinition->toASM();
}

// --- convertASTtoASDL (basic stub) ---

std::unique_ptr<ASDLProgram> convertASTtoASDL(const std::unique_ptr<Program>& ast_program) {
    const auto* prog = ast_program.get();
    const auto* func = prog->function.get();

    std::string funcName = func->name;

    int returnValue = func->body->expression->value;

    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<Mov>(
        std::make_unique<Imm>(returnValue),
        std::make_unique<Register>(Reg::AX)
    ));

    instructions.push_back(std::make_unique<Ret>());

    auto funcDef = std::make_unique<FunctionDefinition>(funcName, std::move(instructions));
    return std::make_unique<ASDLProgram>(std::move(funcDef));
}

// --- convertTackyToASDL
ASDLProgram convertTackyToASDL(const tacky::Program& tackyProgram) {
    std::vector<std::unique_ptr<Instruction>> asdlInstructions;

    for (const auto& instr : tackyProgram.function->body) {
        // Handle Return
        if (auto ret = dynamic_cast<const tacky::Return*>(instr.get())) {
            if (auto c = dynamic_cast<const tacky::Constant*>(ret->value.get())) {
                asdlInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Imm>(c->value),
                    std::make_unique<Register>(Reg::AX)
                ));
            } else if (auto v = dynamic_cast<const tacky::Var*>(ret->value.get())) {
                asdlInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Pseudo>(v->name),
                    std::make_unique<Register>(Reg::AX)
                ));
            }
            asdlInstructions.push_back(std::make_unique<Ret>());
        }

        // Handle Unary
        else if (auto unary = dynamic_cast<const tacky::Unary*>(instr.get())) {
            UnaryOperator op;
            switch (unary->op) {
                case tacky::UnaryOp::Complement: op = UnaryOperator::NOT; break;
                case tacky::UnaryOp::Negate:     op = UnaryOperator::NEG; break;
                default: throw std::runtime_error("Unknown UnaryOp");
            }

            std::unique_ptr<Operand> src;
            if (auto c = dynamic_cast<const tacky::Constant*>(unary->src.get())) {
                src = std::make_unique<Imm>(c->value);
            } else if (auto v = dynamic_cast<const tacky::Var*>(unary->src.get())) {
                src = std::make_unique<Pseudo>(v->name);
            }

            std::string dstName = dynamic_cast<const tacky::Var*>(unary->dst.get())->name;
            auto dst = std::make_unique<Pseudo>(dstName);

            // First: mov src, dst
            asdlInstructions.push_back(std::make_unique<Mov>(
                std::move(src),
                std::make_unique<Pseudo>(dst->getIdentifier())
            ));

            // Then: unary op dst
            asdlInstructions.push_back(std::make_unique<Unary>(
                op,
                std::move(dst)
            ));
        }

        // Handle Binary
        else if (auto binary = dynamic_cast<const tacky::Binary*>(instr.get())) {
            std::unique_ptr<Operand> src1;
            if (auto c = dynamic_cast<const tacky::Constant*>(binary->src1.get())) {
                src1 = std::make_unique<Imm>(c->value);
            } else if (auto v = dynamic_cast<const tacky::Var*>(binary->src1.get())) {
                src1 = std::make_unique<Pseudo>(v->name);
            }

            std::unique_ptr<Operand> src2;
            if (auto c = dynamic_cast<const tacky::Constant*>(binary->src2.get())) {
                src2 = std::make_unique<Imm>(c->value);
            } else if (auto v = dynamic_cast<const tacky::Var*>(binary->src2.get())) {
                src2 = std::make_unique<Pseudo>(v->name);
            }

            std::string dstName = dynamic_cast<const tacky::Var*>(binary->dst.get())->name;
            auto dst = std::make_unique<Pseudo>(dstName);

            if (binary->op == tacky::BinaryOp::DIVIDE || binary->op == tacky::BinaryOp::REMAINDER) {
                // mov src1, %eax
                asdlInstructions.push_back(std::make_unique<Mov>(
                    std::move(src1),
                    std::make_unique<Register>(Reg::AX)
                ));

                // cdq
                asdlInstructions.push_back(std::make_unique<Cdq>());

                // idiv src2
                asdlInstructions.push_back(std::make_unique<Idiv>(
                    std::move(src2)
                ));

                // mov %eax or %edx -> dst
                asdlInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Register>(
                        binary->op == tacky::BinaryOp::DIVIDE ? Reg::AX : Reg::DX
                    ),
                    std::move(dst)
                ));
            } else {
                BinaryOperator op;
                switch (binary->op) {
                    case tacky::BinaryOp::ADD:      op = BinaryOperator::ADD; break;
                    case tacky::BinaryOp::SUBTRACT: op = BinaryOperator::SUB; break;
                    case tacky::BinaryOp::MULTIPLY: op = BinaryOperator::MULT; break;
                    default: throw std::runtime_error("Unknown BinaryOp");
                }

                // mov src1, dst
                asdlInstructions.push_back(std::make_unique<Mov>(
                    std::move(src1),
                    std::make_unique<Pseudo>(dst->getIdentifier())
                ));

                // op src2, dst
                asdlInstructions.push_back(std::make_unique<Binary>(
                    op,
                    std::move(src2),
                    std::move(dst)
                ));
            }
        }
    }

    auto funcDef = std::make_unique<FunctionDefinition>(
        tackyProgram.function->name,
        std::move(asdlInstructions)
    );

    return ASDLProgram(std::move(funcDef));
}

FunctionDefinition* ASDLProgram::getFunctionDefinition() const {
    return functionDefinition.get();
}


int replacePseudosWithStack(ASDLProgram& program) {
    int stackOffset = -4;
    std::unordered_map<std::string, int> pseudoOffsets;

    auto& instructions = program.getFunctionDefinition()->getInstructions();

    for (auto& instr : instructions) {
        // Handle Unary instructions
        if (auto unary = dynamic_cast<Unary*>(instr.get())) {
            const Operand* dstOp = unary->getDst();
            if (auto pseudo = dynamic_cast<const Pseudo*>(dstOp)) {
                const std::string& name = pseudo->getIdentifier();
                if (!pseudoOffsets.count(name)) {
                    pseudoOffsets[name] = stackOffset;
                    stackOffset -= 4;
                }
                unary->setDst(std::make_unique<Stack>(pseudoOffsets[name]));
            }
        }

        // Handle Mov instructions
        if (auto mov = dynamic_cast<Mov*>(instr.get())) {
            Operand* dstOp = mov->getDst();
            if (auto pseudo = dynamic_cast<Pseudo*>(dstOp)) {
                const std::string& name = pseudo->getIdentifier();
                if (!pseudoOffsets.count(name)) {
                    pseudoOffsets[name] = stackOffset;
                    stackOffset -= 4;
                }
                mov->setDst(std::make_unique<Stack>(pseudoOffsets[name]));
            }

            Operand* srcOp = mov->getSrc();
            if (auto pseudo = dynamic_cast<Pseudo*>(srcOp)) {
                const std::string& name = pseudo->getIdentifier();
                if (!pseudoOffsets.count(name)) {
                    pseudoOffsets[name] = stackOffset;
                    stackOffset -= 4;
                }
                mov->setSrc(std::make_unique<Stack>(pseudoOffsets[name]));
            }
        }

        // Handle Binary instructions
        if (auto binary = dynamic_cast<Binary*>(instr.get())) {
            Operand* dstOp = binary->getDst();
            if (auto pseudo = dynamic_cast<Pseudo*>(dstOp)) {
                const std::string& name = pseudo->getIdentifier();
                if (!pseudoOffsets.count(name)) {
                    pseudoOffsets[name] = stackOffset;
                    stackOffset -= 4;
                }
                binary->setDst(std::make_unique<Stack>(pseudoOffsets[name]));
            }

            Operand* srcOp = binary->getSrc();
            if (auto pseudo = dynamic_cast<Pseudo*>(srcOp)) {
                const std::string& name = pseudo->getIdentifier();
                if (!pseudoOffsets.count(name)) {
                    pseudoOffsets[name] = stackOffset;
                    stackOffset -= 4;
                }
                binary->setSrc(std::make_unique<Stack>(pseudoOffsets[name]));
            }
        }

        // Handle Idiv instructions
        if (auto idiv = dynamic_cast<Idiv*>(instr.get())) {
            Operand* dstOp = idiv->getDst();
            if (auto pseudo = dynamic_cast<Pseudo*>(dstOp)) {
                const std::string& name = pseudo->getIdentifier();
                if (!pseudoOffsets.count(name)) {
                    pseudoOffsets[name] = stackOffset;
                    stackOffset -= 4;
                }
                idiv->setDst(std::make_unique<Stack>(pseudoOffsets[name]));
            }
        }
    }

    return stackOffset;
}


std::vector<std::unique_ptr<Instruction>>& FunctionDefinition::getInstructions() {
    return instructions;
}


void insertAllocateStack(ASDLProgram& program, int stackSize) {
    FunctionDefinition* funcDef = program.getFunctionDefinition();
    auto& instructions = funcDef->getInstructions();

    auto allocate = std::make_unique<AllocateStack>(-stackSize);
    instructions.insert(instructions.begin(), std::move(allocate));
}

void legalizeMovMemoryToMemory(ASDLProgram& program) {
    auto& instructions = program.getFunctionDefinition()->getInstructions();
    std::vector<std::unique_ptr<Instruction>> legalizedInstructions;

    for (auto& instr : instructions) {
        if (auto mov = dynamic_cast<Mov*>(instr.get())) {
            Operand* src = mov->getSrc();
            Operand* dst = mov->getDst();

            bool srcIsMemory = dynamic_cast<Stack*>(src) != nullptr;
            bool dstIsMemory = dynamic_cast<Stack*>(dst) != nullptr;

            if (srcIsMemory && dstIsMemory) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(src->clone()),
                    std::make_unique<Register>(Reg::R10)
                ));
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Register>(Reg::R10),
                    std::unique_ptr<Operand>(dst->clone())
                ));
            } else {
                legalizedInstructions.push_back(std::move(instr));
            }

        } else if (auto idiv = dynamic_cast<Idiv*>(instr.get())) {
            Operand* op = idiv->getDst();  // utiliser getDst()

            // Si opérande immédiate, légaliser
            if (dynamic_cast<Imm*>(op)) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(op->clone()),
                    std::make_unique<Register>(Reg::R10)
                ));
                legalizedInstructions.push_back(std::make_unique<Idiv>(
                    std::make_unique<Register>(Reg::R10)
                ));
            } else {
                legalizedInstructions.push_back(std::move(instr));
            }

        } else if (auto bin = dynamic_cast<Binary*>(instr.get())) {
            Operand* src = bin->getSrc();
            Operand* dst = bin->getDst();

            bool srcIsStack = dynamic_cast<Stack*>(src) != nullptr;
            bool dstIsStack = dynamic_cast<Stack*>(dst) != nullptr;
            bool srcIsImm = dynamic_cast<Imm*>(src) != nullptr;

            auto op = bin->getBinaryOperator();  // Il faut ajouter ce getter dans Binary !

            // addl || subl -4(%rbp), -8(%rbp)
            if ((op == BinaryOperator::ADD || op == BinaryOperator::SUB) && srcIsStack && dstIsStack) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(src->clone()),
                    std::make_unique<Register>(Reg::R10)
                ));
                legalizedInstructions.push_back(std::make_unique<Binary>(
                    BinaryOperator::ADD,
                    std::make_unique<Register>(Reg::R10),
                    std::unique_ptr<Operand>(dst->clone())
                ));

            // imull $3, -4(%rbp)
            } else if (op == BinaryOperator::MULT && srcIsImm && dstIsStack) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(dst->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Binary>(
                    BinaryOperator::MULT,
                    std::unique_ptr<Operand>(src->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Register>(Reg::R11),
                    std::unique_ptr<Operand>(dst->clone())
                ));
            } else {
                legalizedInstructions.push_back(std::move(instr));
            }

        } else {
            legalizedInstructions.push_back(std::move(instr));
        }
    }

    instructions = std::move(legalizedInstructions);
}

// --- writeASMToFile ---

void writeASMToFile(const ASDLProgram& program, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open output file: " + filename);
    }

    ofs << program.toASM();
    ofs.close();
}
