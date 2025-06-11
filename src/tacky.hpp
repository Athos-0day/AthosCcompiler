#ifndef TACKY_HPP
#define TACKY_HPP

#include <string>
#include <vector>
#include <memory>

namespace tacky {

/**
 * @brief Base class for all value types in the TACKY IR.
 */
struct Val {
    virtual ~Val() = default;

    /**
     * @brief Converts the value to a string representation.
     * @return A string representation of the value.
     */
    virtual std::string toString() const = 0;
};

/**
 * @brief Represents an integer constant value.
 */
struct Constant : Val {
    int value;

    /**
     * @brief Constructs a Constant value.
     * @param v The integer constant.
     */
    Constant(int v) : value(v) {}

    std::string toString() const override;
};

/**
 * @brief Represents a variable by its name.
 */
struct Var : Val {
    std::string name;

    /**
     * @brief Constructs a Var with a given identifier.
     * @param id The name of the variable.
     */
    Var(const std::string& id) : name(id) {}

    std::string toString() const override;
};

/**
 * @brief Supported unary operators in the TACKY IR.
 */
enum class UnaryOp {
    Complement,  ///< Bitwise complement (~)
    Negate       ///< Arithmetic negation (-)
};

/**
 * @brief Base class for all instructions in TACKY IR.
 */
struct Instruction {
    virtual ~Instruction() = default;

    /**
     * @brief Converts the instruction to a string representation.
     * @return A string representation of the instruction.
     */
    virtual std::string toString() const = 0;
};

/**
 * @brief Represents a return instruction.
 */
struct Return : Instruction {
    std::unique_ptr<Val> value;

    /**
     * @brief Constructs a Return instruction.
     * @param v The value to return.
     */
    Return(std::unique_ptr<Val> v) : value(std::move(v)) {}

    std::string toString() const override;
};

/**
 * @brief Represents a unary operation instruction.
 */
struct Unary : Instruction {
    UnaryOp op;                        ///< The unary operator
    std::unique_ptr<Val> src;         ///< Source operand
    std::unique_ptr<Val> dst;         ///< Destination variable

    /**
     * @brief Constructs a Unary instruction.
     * @param o The unary operator.
     * @param s The operand value.
     * @param d The destination variable.
     */
    Unary(UnaryOp o, std::unique_ptr<Val> s, std::unique_ptr<Val> d)
        : op(o), src(std::move(s)), dst(std::move(d)) {}

    std::string toString() const override;
};

/**
 * @brief Represents a function in the TACKY IR.
 */
struct Function {
    std::string name; ///< Name of the function
    std::vector<std::unique_ptr<Instruction>> body; ///< List of instructions

    /**
     * @brief Constructs a Function.
     * @param n The name of the function.
     */
    Function(const std::string& n) : name(n) {}

    /**
     * @brief Converts the function to a string representation.
     * @return A string representation of the function and its instructions.
     */
    std::string toString() const;
};

/**
 * @brief Represents the top-level program in TACKY IR.
 */
struct Program {
    std::unique_ptr<Function> function;

    /**
     * @brief Constructs a Program.
     * @param f The function definition.
     */
    Program(std::unique_ptr<Function> f) : function(std::move(f)) {}

    /**
     * @brief Converts the program to a string representation.
     * @return A string representation of the program.
     */
    std::string toString() const;
};

} // namespace tacky

#endif // TACKY_HPP
