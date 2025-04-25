#!/usr/bin/env python3
import sys
import os
from lexer import lexer, LexerError
from parser import Parser
from ast import Program
from codegen import generate_code, compile_assembler

def print_tokens(tokens):
    print("\nLexer output:")
    print(f"{'Type':<15} {'Value':<30} {'Position'}")
    print("-" * 50)
    for token in tokens:
        print(f"{token.type:<15} {token.value:<30} {token.position}")

def print_help():
    print("Usage:")
    print("  ./compiler.py --lex <source_file>      # Run the lexer only")
    print("  ./compiler.py --parse <source_file>    # Run the lexer and parser, then show AST")
    print("  ./compiler.py --codegen <source_file>  # Generate assembly from parsed AST")
    print("  ./compiler.py <source_file>            # Compile and link (default behavior)")
    print("  ./compiler.py --help                   # Show this help message")

def main():
    if len(sys.argv) == 2:
        # Si juste ./compiler.py test.c => par défaut faire compile
        mode = "--compile"
        filepath = sys.argv[1]
    elif len(sys.argv) == 3:
        mode, filepath = sys.argv[1], sys.argv[2]
    elif len(sys.argv) == 2 and sys.argv[1] == "--help":
        print_help()
        sys.exit(0)
    else:
        print_help()
        sys.exit(1)

    if not os.path.isfile(filepath):
        print(f"Error: file not found -> {filepath}")
        sys.exit(1)

    try:
        with open(filepath, 'r') as f:
            code = f.read()

        tokens = lexer(code)

        if mode == "--lex":
            print_tokens(tokens)
        elif mode == "--parse":
            parser = Parser(tokens)
            ast = parser.parse_program()
            print("Parser output (AST):")
            print(ast)
        elif mode == "--codegen":
            parser = Parser(tokens)
            ast = parser.parse_program()
            code = generate_code(ast)

            with open("out.s", "w") as f:
                f.write(code)
            print("Assembly code written to out.s")
        elif mode == "--compile":
            parser = Parser(tokens)
            ast = parser.parse_program()
            asm_code = generate_code(ast)

            asm_filename = "out.s"
            with open(asm_filename, "w") as f:
                f.write(asm_code)

            dirpath = os.path.dirname(filepath)
            basename = os.path.splitext(os.path.basename(filepath))[0]
            output_path = os.path.join(dirpath, basename)
            compile_assembler(asm_filename, output_path)
        else:
            print(f"Unknown mode: {mode}")
            print_help()
            sys.exit(1)

    except LexerError as e:
        print(f"Lexer error: {e}")
        sys.exit(1)
    except SyntaxError as e:
        print(f"Parser error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
