#include "asdl.hpp"
#include <fstream>
#include <stdexcept>
#include <cctype>  // for std::tolower

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
    std::string regName = regToString(reg);
    for (auto& c : regName) c = static_cast<char>(std::tolower(c));
    return "%" + regName;
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
    return "movq " + src->toASM() + ", " + dst->toASM();
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
            return "negq " + dst->toASM();
        case UnaryOperator::NOT:
            return "notq " + dst->toASM();
        default:
            return "UNKNOWN_UNARY_OP";
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
    return "ret";
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
    std::string result = "FunctionDefinition(name=" + name + ", instructions=[";
    for (size_t i = 0; i < instructions.size(); ++i) {
        result += instructions[i]->toString();
        if (i + 1 < instructions.size())
            result += ", ";
    }
    result += "])";
    return result;
}

std::string FunctionDefinition::toASM() const {
    std::string asmCode = ".globl " + name + "\n" + name + ":\n";
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
        } else if (auto unary = dynamic_cast<const tacky::Unary*>(instr.get())) {
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
    }

    auto funcDef = std::make_unique<FunctionDefinition>(
        tackyProgram.function->name,
        std::move(asdlInstructions)
    );

    return ASDLProgram(std::move(funcDef));
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
