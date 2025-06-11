#ifndef ASDL_HPP
#define ASDL_HPP

#include <string>
#include <memory>
#include <vector>

#include "ast.hpp"
#include "tacky.hpp"

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
 * @brief Enum representing registers.
 */
enum class Reg {
    AX,
    R10
};

/**
 * @brief Enum representing unary-operators.
 */
enum class UnaryOperator {
    NEG,
    NOT
};

/**
 * @brief Convert Reg enum to string.
 */
inline std::string regToString(Reg r) {
    switch (r) {
        case Reg::AX: return "AX";
        case Reg::R10: return "R10";
        default: return "UNKNOWN_REG";
    }
}

/**
 * @brief Operand type: immediate value or register.
 */
class Operand : public ASDLNode {
public:
    virtual ~Operand() = default;

    /**
     * @brief Clone the operand object.
     * @return A dynamically allocated copy of the operand.
     */
    virtual Operand* clone() const = 0;
};

class Imm : public Operand {
    int value;
public:
    explicit Imm(int v) : value(v) {}
    int getValue() const { return value; }
    Operand* clone() const override { return new Imm(value); }

    std::string toString() const override;
    std::string toASM() const override;
};

class Register : public Operand {
    Reg reg;
public:
    explicit Register(Reg r) : reg(r) {}
    Reg getReg() const { return reg; }
    Operand* clone() const override { return new Register(reg); }

    std::string toString() const override;
    std::string toASM() const override;
};

class Pseudo : public Operand {
    std::string identifier;
public:
    explicit Pseudo(std::string id) : identifier(std::move(id)) {}
    const std::string& getIdentifier() const { return identifier; }
    Operand* clone() const override { return new Pseudo(identifier); }

    std::string toString() const override;
    std::string toASM() const override;
};

class Stack : public Operand {
    int value;
public:
    explicit Stack(int n) : value(n) {}
    int getValue() const { return value; }
    Operand* clone() const override { return new Stack(value); }

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
    Operand* getDst() {return dst.get();}
    Operand* getSrc() {return src.get();}

    Operand* cloneSrc() const { return src->clone(); }
    Operand* cloneDst() const { return dst->clone(); }



    void setDst(std::unique_ptr<Operand> newDst) { dst = std::move(newDst); }
    void setSrc(std::unique_ptr<Operand> newSrc) { src = std::move(newSrc); }

    std::string toString() const override;
    std::string toASM() const override;
};

class Unary : public Instruction {
    UnaryOperator unary_operator;
    std::unique_ptr<Operand> dst;
public:
    Unary(UnaryOperator u, std::unique_ptr<Operand> d);

    std::string toString() const override;
    std::string toASM() const override;

    Operand* getDst() { return dst.get(); }
    void setDst(std::unique_ptr<Operand> newDst) { dst = std::move(newDst); }
};

class AllocateStack : public Instruction {
    int value;
public:
    explicit AllocateStack(int n);

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
    std::vector<std::unique_ptr<Instruction>>& getInstructions();
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
    FunctionDefinition* getFunctionDefinition() const;

};

/** 
 * @brief convert a tacky program to an ASDL program.
 *
 * @param program The tacky program.
 * @return The ASDL program.
 */
ASDLProgram convertTackyToASDL(const tacky::Program& tackyProgram);

/**
 * @brief Replaces all Pseudo operands in the ASDL IR with stack-based operands.
 *
 * This function traverses the ASDL program and replaces each `Pseudo` operand 
 * with a `Stack` operand representing a memory location on the stack. 
 * Each unique `Pseudo` variable is assigned a unique negative offset from `rbp`, 
 * starting from -4 and decrementing by 4 for each additional temporary variable.
 *
 * This transformation is necessary to allocate space for temporaries in the 
 * function's stack frame during code generation.
 *
 * @param program A unique pointer to the ASDLProgram to be transformed.
 * @return stack offset
 */
int replacePseudosWithStack(ASDLProgram& program);

/**
 * @brief Inserts an AllocateStack instruction at the beginning of the ASDL program.
 *
 * This is typically used to allocate space on the stack for local variables or pseudo-registers.
 *
 * @param program The ASDL program to modify.
 * @param stackSize The size (in bytes) to allocate on the stack.
 */
void insertAllocateStack(ASDLProgram& program, int stackSize);

/**
 * @brief Legalizes memory-to-memory move instructions in the ASDLProgram.
 *
 * x86 assembly does not allow instructions like `movl -4(%rbp), -8(%rbp)`,
 * which move data directly from one memory location to another.
 *
 * This function traverses the ASDLProgram, and for every invalid `Mov`
 * instruction of the form `movl memory, memory`, it replaces it with:
 *
 *     movl source_mem, %r10d
 *     movl %r10d, dest_mem
 *
 * @param program The ASDLProgram to be transformed.
 */
void legalizeMovMemoryToMemory(ASDLProgram& program);


/**
 * @brief Write the assembly code generated from the ASDL Program to a file.
 *
 * @param program The ASDL Program node.
 * @param filename Output filename.
 */
void writeASMToFile(const ASDLProgram& program, const std::string& filename);

#endif // ASDL_HPP
