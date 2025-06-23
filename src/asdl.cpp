#include "asdl.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cctype>  // for std::tolower

// --- Idiv ---
Idiv::Idiv(std::unique_ptr<Operand> d) : dst(std::move(d)) {}

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


// --- Jmp ---
std::string Jmp::toString() const {
    return "Jmp(id= " + name + ")";
}

std::string Jmp::toASM() const {
    return "jmp L" + name;
}

Jmp::Jmp(std::string target) : name(std::move(target)) {
}

// --- JmpCC ---
std::string JmpCC::toString() const {
    return "JmpCC(cond=" + condNodeToString(cond_node) + ", id=" + name + ")";
}

std::string JmpCC::toASM() const {
    return "j" + condNodeToASM(cond_node) + " L" + name;
}

JmpCC::JmpCC(CondNode cond, std::string target) : cond_node(cond), name(std::move(target)) {
}

// --- Cmp ---
std::string Cmp::toString() const {
    return "Cmp(e1=" + rhs->toString() + ", e2=" + lhs->toString() + ")";
}

std::string Cmp::toASM() const {
    return "cmpl " + rhs->toASM() + ", " + lhs->toASM();
}

// --- Idiv ---
std::string Idiv::toString() const {
    return "Idiv(dst= " + dst->toString() + ")";
}

std::string Idiv::toASM() const {
    return "idivl " + dst->toASM();
}

// --- SetCC ---
std::string SetCC::toString() const {
    return "SetCC(cond=" + condNodeToString(cond_node) + ", op=" + op->toString() + ")";
}

std::string SetCC::toASM() const {
    return "set" + condNodeToASM(cond_node) + " " + op->toASM();
}

SetCC::SetCC(CondNode cond, std::unique_ptr<Operand> dst)
    : cond_node(cond), op(std::move(dst)) {}

// --- Label ---
std::string Label::toString() const {
    return "Label(" + name + ")";
}

std::string Label::toASM() const {
    return "L" + name + ":";
}

Label::Label(std::string name) : Instruction(), name(std::move(name)) {
}

// --- Binary ---
std::string Binary::toString() const {
    std::string opStr;
    if (isBinaryOperator() && (getBinaryOperator() == BinaryOperator::ADD)) {
        opStr = "ADD";
    } else if (isBinaryOperator() && (getBinaryOperator() == BinaryOperator::SUB)) {
        opStr = "SUB";
    } else {
        opStr = "MULT";
    }

    return "Binary(" + opStr + ", " + src->toString() + ", " + dst->toString() + ")";
}

std::string Binary::toASM() const {
    switch (getBinaryOperator()) {
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
    asmCode += "  pushq %rbp\n";
    asmCode += "  movq %rsp, %rbp\n";

    for (const auto& instr : instructions) {
        if (dynamic_cast<Label*>(instr.get())) {
            // Saut de ligne avant le label
            asmCode += "\n";
            // Label sans indentation
            asmCode += instr->toASM() + "\n";
            // Saut de ligne après le label
            asmCode += "\n";
        } else {
            // Instruction classique avec indentation
            asmCode += "  " + instr->toASM() + "\n";
        }
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

std::unique_ptr<Operand> convertValToOperand(const std::unique_ptr<tacky::Val>& val) {
    if (auto c = dynamic_cast<const tacky::Constant*>(val.get())) {
        return std::make_unique<Imm>(c->value);
    } else if (auto v = dynamic_cast<const tacky::Var*>(val.get())) {
        return std::make_unique<Pseudo>(v->name);
    } else {
        throw std::runtime_error("Unsupported tacky::Val type");
    }
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

        //Handle Jmp
        else if (auto jmp = dynamic_cast<const tacky::Jump*>(instr.get())) {
            asdlInstructions.push_back(std::make_unique<Jmp>(jmp->target));
        }

        //Handle JmpIfZero
        else if (auto jmpiz = dynamic_cast<const tacky::JumpIfZero*>(instr.get())) {
            asdlInstructions.push_back(std::make_unique<Cmp>(
                std::make_unique<Imm>(0),
                convertValToOperand(jmpiz->condition)
            ));

            asdlInstructions.push_back(std::make_unique<JmpCC>(
                CondNode::E,
                jmpiz->target
            ));
        }

        //Handle JmpIfNotZero
        else if (auto jmpinz = dynamic_cast<const tacky::JumpIfNotZero*>(instr.get())) {
            asdlInstructions.push_back(std::make_unique<Cmp>(
                std::make_unique<Imm>(0),
                convertValToOperand(jmpinz->condition)
            ));

            asdlInstructions.push_back(std::make_unique<JmpCC>(
                CondNode::NE,
                jmpinz->target
            ));
        }

        //Handle Copy
        else if (auto copy = dynamic_cast<const tacky::Copy*>(instr.get())) {
            asdlInstructions.push_back(std::make_unique<Mov>(
                convertValToOperand(copy->src),
                convertValToOperand(copy->dst)
            ));
        }

        //Handle Label
        else if (auto label = dynamic_cast<const tacky::Label*>(instr.get())) {
            asdlInstructions.push_back(std::make_unique<Label>(
                label->name
            ));
        }

        // Handle Unary
        else if (auto unary = dynamic_cast<const tacky::Unary*>(instr.get())) {
            UnaryOperator op;

            if (unary->op == tacky::UnaryOp::Not) {
                asdlInstructions.push_back(std::make_unique<Cmp>(
                    std::make_unique<Imm>(0),
                    convertValToOperand(unary->src)
                ));

                asdlInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Imm>(0),
                    convertValToOperand(unary->dst)
                ));

                asdlInstructions.push_back(std::make_unique<SetCC>(
                    CondNode::E,
                    convertValToOperand(unary->dst)
                ));
            } else {
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
            } else if (binary->op == tacky::BinaryOp::ADD || binary->op == tacky::BinaryOp::SUBTRACT || binary->op == tacky::BinaryOp::MULTIPLY) {
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
            } else {
                CondNode op;
                switch (binary->op) {
                    case tacky::BinaryOp::EQUAL:      op = CondNode::E; break;
                    case tacky::BinaryOp::NOTEQUAL: op = CondNode::NE; break;
                    case tacky::BinaryOp::LESSTHAN: op = CondNode::L; break;
                    case tacky::BinaryOp::LESSEQ:      op = CondNode::LE; break;
                    case tacky::BinaryOp::GREATERTHAN: op = CondNode::G; break;
                    case tacky::BinaryOp::GREATEREQ: op = CondNode::GE; break;
                    default: throw std::runtime_error("Unknown RelationOp in BinaryOp");
                }

                asdlInstructions.push_back(std::make_unique<Cmp>(
                    std::move(src2),
                    std::move(src1)
                ));

                asdlInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Imm>(0),
                    convertValToOperand(binary->dst)
                ));

                asdlInstructions.push_back(std::make_unique<SetCC>(
                    op,
                    convertValToOperand(binary->dst)
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

std::unique_ptr<Operand> replaceIfPseudo(Operand* op, std::unordered_map<std::string, int>& offsets, int& stackOffset) {
    if (auto pseudo = dynamic_cast<Pseudo*>(op)) {
        const std::string& name = pseudo->getIdentifier();
        if (!offsets.count(name)) {
            offsets[name] = stackOffset;
            stackOffset -= 4;
        }
        return std::make_unique<Stack>(offsets[name]);
    }
    return nullptr;
}

int replacePseudosWithStack(ASDLProgram& program) {
    int stackOffset = -4;
    std::unordered_map<std::string, int> pseudoOffsets;
    auto& instructions = program.getFunctionDefinition()->getInstructions();

    for (auto& instr : instructions) {
        if (auto unary = dynamic_cast<Unary*>(instr.get())) {
            auto dst = unary->releaseDst();
            if (auto replaced = replaceIfPseudo(dst.get(), pseudoOffsets, stackOffset))
                unary->setDst(std::move(replaced));
            else
                unary->setDst(std::move(dst));
        }

        else if (auto mov = dynamic_cast<Mov*>(instr.get())) {
            auto dst = mov->releaseDst();
            auto src = mov->releaseSrc();

            if (auto replaced = replaceIfPseudo(dst.get(), pseudoOffsets, stackOffset))
                mov->setDst(std::move(replaced));
            else
                mov->setDst(std::move(dst));

            if (auto replaced = replaceIfPseudo(src.get(), pseudoOffsets, stackOffset))
                mov->setSrc(std::move(replaced));
            else
                mov->setSrc(std::move(src));
        }

        else if (auto binary = dynamic_cast<Binary*>(instr.get())) {
            auto dst = binary->releaseDst();
            auto src = binary->releaseSrc();

            if (auto replaced = replaceIfPseudo(dst.get(), pseudoOffsets, stackOffset))
                binary->setDst(std::move(replaced));
            else
                binary->setDst(std::move(dst));

            if (auto replaced = replaceIfPseudo(src.get(), pseudoOffsets, stackOffset))
                binary->setSrc(std::move(replaced));
            else
                binary->setSrc(std::move(src));
        }

        else if (auto cmp = dynamic_cast<Cmp*>(instr.get())) {
            auto lhs = cmp->releaseLHS();
            auto rhs = cmp->releaseRHS();

            if (auto replaced = replaceIfPseudo(lhs.get(), pseudoOffsets, stackOffset))
                cmp->setLHS(std::move(replaced));
            else
                cmp->setLHS(std::move(lhs));

            if (auto replaced = replaceIfPseudo(rhs.get(), pseudoOffsets, stackOffset))
                cmp->setRHS(std::move(replaced));
            else
                cmp->setRHS(std::move(rhs));
        }

        else if (auto setcc = dynamic_cast<SetCC*>(instr.get())) {
            auto dst = setcc->releaseDst();
            if (auto replaced = replaceIfPseudo(dst.get(), pseudoOffsets, stackOffset))
                setcc->setDst(std::move(replaced));
            else
                setcc->setDst(std::move(dst));
        }

        else if (auto idiv = dynamic_cast<Idiv*>(instr.get())) {
            auto dst = idiv->releaseDst();
            if (auto replaced = replaceIfPseudo(dst.get(), pseudoOffsets, stackOffset))
                idiv->setDst(std::move(replaced));
            else
                idiv->setDst(std::move(dst));
        }
    }

    return -stackOffset; 
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

            bool srcIsMem = dynamic_cast<Stack*>(src);
            bool dstIsMem = dynamic_cast<Stack*>(dst);

            if (srcIsMem && dstIsMem) {
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
            Operand* operand = idiv->getDst();

            if (dynamic_cast<Imm*>(operand)) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(operand->clone()),
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
            BinaryOperator op = bin->getBinaryOperator();

            bool srcIsMem = dynamic_cast<Stack*>(src);
            bool dstIsMem = dynamic_cast<Stack*>(dst);
            bool srcIsImm = dynamic_cast<Imm*>(src);

            if ((op == BinaryOperator::ADD || op == BinaryOperator::SUB) && srcIsMem && dstIsMem) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(src->clone()),
                    std::make_unique<Register>(Reg::R10)
                ));
                legalizedInstructions.push_back(std::make_unique<Binary>(
                    op,
                    std::make_unique<Register>(Reg::R10),
                    std::unique_ptr<Operand>(dst->clone())
                ));
            } else if (op == BinaryOperator::MULT && srcIsImm && dstIsMem) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(dst->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Binary>(
                    op,
                    std::unique_ptr<Operand>(src->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Register>(Reg::R11),
                    std::unique_ptr<Operand>(dst->clone())
                ));
            } else if (op == BinaryOperator::MULT && srcIsMem && dstIsMem) {
                // Ex: imull -8(%rbp), -12(%rbp) → temp = [dst]; temp *= [src]; dst = temp
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(dst->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(src->clone()),
                    std::make_unique<Register>(Reg::R10)
                ));
                legalizedInstructions.push_back(std::make_unique<Binary>(
                    op,
                    std::make_unique<Register>(Reg::R10),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::make_unique<Register>(Reg::R11),
                    std::unique_ptr<Operand>(dst->clone())
                ));
            }
            else {
                legalizedInstructions.push_back(std::move(instr));
            }

        } else if (auto cmp = dynamic_cast<Cmp*>(instr.get())) {
            Operand* lhs = cmp->getLHS();
            Operand* rhs = cmp->getRHS();

            bool lhsIsMem = dynamic_cast<Stack*>(lhs);
            bool rhsIsMem = dynamic_cast<Stack*>(rhs);
            bool lhsIsImm = dynamic_cast<Imm*>(lhs);
            bool rhsIsImm = dynamic_cast<Imm*>(rhs);

            if (lhsIsMem && rhsIsMem) {
                // mem vs mem → use temp register
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(lhs->clone()),
                    std::make_unique<Register>(Reg::R10)
                ));
                legalizedInstructions.push_back(std::make_unique<Cmp>(
                    std::make_unique<Register>(Reg::R10),
                    std::unique_ptr<Operand>(rhs->clone())
                ));
            } else if (lhsIsMem && rhsIsImm) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(rhs->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Cmp>(
                    std::unique_ptr<Operand>(lhs->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
            } else if (lhsIsImm && rhsIsMem) {
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(lhs->clone()),
                    std::make_unique<Register>(Reg::R11)
                ));
                legalizedInstructions.push_back(std::make_unique<Cmp>(
                    std::make_unique<Register>(Reg::R11),
                    std::unique_ptr<Operand>(rhs->clone())
                ));
            } else if (lhsIsImm && rhsIsImm) {
                // illegal: imm vs imm, must legalize
                legalizedInstructions.push_back(std::make_unique<Mov>(
                    std::unique_ptr<Operand>(lhs->clone()),
                    std::make_unique<Register>(Reg::R10)
                ));
                legalizedInstructions.push_back(std::make_unique<Cmp>(
                    std::make_unique<Register>(Reg::R10),
                    std::unique_ptr<Operand>(rhs->clone())
                ));
            } else {
                // already legal (e.g., reg vs imm)
                legalizedInstructions.push_back(std::move(instr));
            }

        } else {
            // All other instructions passed through
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

    std::string asmCode = program.toASM();

    // Search for the '_main:' label
    size_t mainPos = asmCode.find("_main:");
    if (mainPos != std::string::npos) {
        // Find where _main ends (next .globl or end of string)
        size_t nextLabelPos = asmCode.find(".globl", mainPos + 1);
        size_t endOfMain = (nextLabelPos != std::string::npos) ? nextLabelPos : asmCode.length();

        // Extract _main body
        std::string mainBody = asmCode.substr(mainPos, endOfMain - mainPos);

        // Check if 'ret' is present
        bool hasRet = mainBody.find("\n  ret") != std::string::npos;
        bool hasRestoreStack = mainBody.find("movq %rbp, %rsp") != std::string::npos &&
                               mainBody.find("popq %rbp") != std::string::npos;

        // If 'ret' is missing, insert proper return sequence before end of _main
        if (!hasRet) {
            std::string retSequence;
            if (!hasRestoreStack) {
                retSequence += "  movq %rbp, %rsp\n  popq %rbp\n";
            }
            retSequence += "  movl $0, %eax\n  ret\n";

            // Insert before end of _main block
            size_t insertPos = asmCode.rfind('\n', endOfMain);
            asmCode.insert(insertPos, "\n" + retSequence);
        }
    } else {
        // Fallback: append full return sequence
        asmCode.append("\n  movq %rbp, %rsp\n  popq %rbp\n  movl $0, %eax\n  ret");
    }

    ofs << asmCode;
    ofs.close();
}


