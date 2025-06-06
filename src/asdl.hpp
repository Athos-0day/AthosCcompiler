#ifndef ASDL_HPP
#define ASDL_HPP

#include <string>
#include <memory>
#include <vector>

#include "ast.hpp" 

/**
 * @brief Base class for all ASDL nodes.
 */
class ASDLNode {
public:
    virtual ~ASDLNode() = default;

    /**
     * @brief Convert node to a string representation (debug).
     */
    virtual std::string toString() const = 0;

    /**
     * @brief Convert node to assembly code string.
     */
    virtual std::string toASM() const = 0;
};

/**
 * @brief Operand type: immediate value or register.
 */
class Operand : public ASDLNode {
public:
    virtual ~Operand() = default;
};

class Imm : public Operand {
    int value;
public:
    explicit Imm(int v) : value(v) {}
    int getValue() const { return value; }

    std::string toString() const override;
    std::string toASM() const override;
};

class Register : public Operand {
    std::string name;
public:
    explicit Register(std::string n) : name(std::move(n)) {}
    const std::string& getName() const { return name; }

    std::string toString() const override;
    std::string toASM() const override;
};

/**
 * @brief Instruction type: Mov or Ret.
 */
class Instruction : public ASDLNode {
public:
    virtual ~Instruction() = default;
};

class Mov : public Instruction {
    std::unique_ptr<Operand> src;
    std::unique_ptr<Operand> dst;
public:
    Mov(std::unique_ptr<Operand> s, std::unique_ptr<Operand> d);

    std::string toString() const override;
    std::string toASM() const override;
};

class Ret : public Instruction {
public:
    Ret() = default;

    std::string toString() const override;
    std::string toASM() const override;
};

/**
 * @brief FunctionDefinition contains a function name and a list of instructions.
 */
class FunctionDefinition : public ASDLNode {
    std::string name;
    std::vector<std::unique_ptr<Instruction>> instructions;
public:
    FunctionDefinition(std::string n, std::vector<std::unique_ptr<Instruction>> ins);

    const std::string& getName() const;
    const std::vector<std::unique_ptr<Instruction>>& getInstructions() const;

    std::string toString() const override;
    std::string toASM() const override;
};

/**
 * @brief ASDLProgram node contains a single FunctionDefinition.
 */
class ASDLProgram : public ASDLNode {
    std::unique_ptr<FunctionDefinition> functionDefinition;
public:
    explicit ASDLProgram(std::unique_ptr<FunctionDefinition> funcDef);

    std::string toString() const override;
    std::string toASM() const override;
};

/**
 * @brief Convert an AST Program to an ASDL Program.
 *
 * @param ast_program The AST root node.
 * @return std::unique_ptr<ASDLProgram> ASDL Program node.
 */
std::unique_ptr<ASDLProgram> convertASTtoASDL(const std::unique_ptr<Program>& ast_program);

/**
 * @brief Write the assembly code generated from the ASDL Program to a file.
 *
 * @param program The ASDL Program node.
 * @param filename Output filename.
 */
void writeASMToFile(const ASDLProgram& program, const std::string& filename);

#endif // ASDL_HPP
