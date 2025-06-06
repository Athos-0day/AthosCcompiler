#include "asdl.hpp"
#include <fstream>

// Imm

std::string Imm::toString() const {
    return "Imm(" + std::to_string(value) + ")";
}

std::string Imm::toASM() const {
    return "$" + std::to_string(value);
}

// Register

std::string Register::toString() const {
    return "Register(" + name + ")";
}

std::string Register::toASM() const {
    return "%" + name;
}

// Mov

Mov::Mov(std::unique_ptr<Operand> s, std::unique_ptr<Operand> d)
    : src(std::move(s)), dst(std::move(d)) {}

std::string Mov::toString() const {
    return "Mov(src=" + src->toString() + ", dst=" + dst->toString() + ")";
}

std::string Mov::toASM() const {
    return "movl " + src->toASM() + ", " + dst->toASM();
}

// Ret

std::string Ret::toString() const {
    return "Ret";
}

std::string Ret::toASM() const {
    return "ret";
}

// FunctionDefinition

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
    std::string asmCode = name + ":\n";
    for (const auto& instr : instructions) {
        asmCode += "  " + instr->toASM() + "\n";
    }
    return asmCode;
}

// ASDLProgram

ASDLProgram::ASDLProgram(std::unique_ptr<FunctionDefinition> funcDef)
    : functionDefinition(std::move(funcDef)) {}

std::string ASDLProgram::toString() const {
    return "ASDLProgram(" + functionDefinition->toString() + ")";
}

std::string ASDLProgram::toASM() const {
    return functionDefinition->toASM();
}

// Conversion from AST to ASDL (stub, should be implemented properly)

#include "ast.hpp"  // Assuming ASTNode and its subclasses are defined here
#include <stdexcept>

std::unique_ptr<ASDLProgram> convertASTtoASDL(const std::unique_ptr<Program>& ast_program) {
    const auto* prog = ast_program.get();  // no dynamic_cast needed anymore

    const auto* func = prog->function.get();
    const std::string& funcName = func->name;

    int returnValue = func->body->expression->value;

    std::vector<std::unique_ptr<Instruction>> instructions;
    instructions.push_back(std::make_unique<Mov>(
        std::make_unique<Imm>(returnValue),
        std::make_unique<Register>("eax")
    ));
    instructions.push_back(std::make_unique<Ret>());

    auto functionDefinition = std::make_unique<FunctionDefinition>(
        funcName,
        std::move(instructions)
    );

    return std::make_unique<ASDLProgram>(std::move(functionDefinition));
}


// Write ASM to file

void writeASMToFile(const ASDLProgram& program, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open output file: " + filename);
    }

    ofs << program.toASM();
    ofs.close();
}
