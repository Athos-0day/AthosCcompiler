# codegen.py
import subprocess
import os

def generate_code(ast):
    """
    Generates x86-64 assembly code from the AST.
    Currently only supports:
      - int main(void) { return <constant>; }
    """
    lines = []

    # Only support one top-level function: main
    if ast.function.name != "main":
        raise ValueError("Only 'main' function supported for now.")

    return_stmt = ast.function.body
    if return_stmt.__class__.__name__ != "ReturnStatement":
        raise ValueError("Only return statements are supported.")

    expr = return_stmt.expression
    if expr.__class__.__name__ != "Constant":
        raise ValueError("Only constant return values are supported.")

    value = expr.value

    # Generate minimal x86-64 assembly (MacOScalling convention)
    lines.append("    .globl _main")
    lines.append("_main:")
    lines.append(f"    mov w0, #{value}")  # Return value in EAX
    lines.append("    ret")                   # Return from main

    return "\n".join(lines)

def compile_assembler(asm_filename, output_name):
    """
    Compiles the given assembly file using gcc and removes it after.
    """
    try:
        subprocess.run(["gcc", "-o", output_name, asm_filename], check=True)
        print(f"Executable '{output_name}' created successfully.")
    except subprocess.CalledProcessError as e:
        print("Error during compilation with gcc.")
        raise e
    finally:
        if os.path.exists(asm_filename):
            os.remove(asm_filename)