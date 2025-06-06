/**
 * @file compiler.cpp
 * @brief Main driver for the simple C-like language compiler.
 * 
 * This program serves as the entry point for the compiler pipeline. It supports
 * multiple modes of operation selected via command-line arguments:
 * 
 * - `--lex <source_file>`: Runs the lexer on the source file and prints tokens.
 * - `--parse <source_file>`: Runs the lexer and parser on the source file,
 *    showing verbose parsing logs.
 * - `--codegen <source_file>`: Runs the full pipeline up to code generation
 *    and outputs assembly code.
 * - `<source_file>` (default): Runs the full compile and link pipeline.
 * - `--help`: Prints usage information.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "asdl.hpp"

/**
 * @brief Prints the usage/help message to the console.
 */
void print_help() {
    std::cout << "Usage:\n";
    std::cout << "  ./compiler --lex <source_file>      # Run the lexer only\n";
    std::cout << "  ./compiler --parse <source_file>    # Run the lexer and parser with verbose logs\n";
    std::cout << "  ./compiler --codegen <source_file>  # Generate assembly from parsed AST\n";
    std::cout << "  ./compiler <source_file>             # Compile and link (default behavior)\n";
    std::cout << "  ./compiler --help                   # Show this help message\n";
}

/**
 * @brief Entry point for the compiler program.
 * 
 * Parses command line arguments to determine the mode of operation and source file.
 * Dispatches the compilation phases accordingly: lexing, parsing, codegen, or full compile.
 * Catches and reports runtime errors, including lexical and parse errors.
 * 
 * @param argc Number of command line arguments.
 * @param argv Array of command line argument strings.
 * @return int Exit status code (0 on success, non-zero on error).
 */
int main(int argc, char* argv[]) {
    std::string filepath;
    std::string mode;

    // Analyse des arguments de la ligne de commande
    if (argc == 2) {
        if (std::string(argv[1]) == "--help") {
            print_help();
            return 0;
        }
        mode = "--compile";  // comportement par défaut
        filepath = argv[1];
    } else if (argc == 3) {
        mode = argv[1];
        filepath = argv[2];
    } else {
        print_help();
        return 1;
    }

    try {
        if (mode == "--lex") {
            std::cout << "Running lexer on: " << filepath << "\n";
            auto lex = lexer(filepath, true);
            if (lex.empty()) {
                std::cerr << "Lexer returned no tokens.\n";
                return 1;
            }
        } else if (mode == "--parse") {
            std::cout << "Running lexer and parser on: " << filepath << "\n";
            auto lex = lexer(filepath, false);
            Parser parser(lex, true);  // verbose enabled to see logs
            auto ast = parser.parseProgram();
            std::cout << "Parsing completed successfully.\n";

        } else if (mode == "--codegen") {
            std::cout << "Generating assembly from: " << filepath << "\n";

            // 1) Run the lexer on the source file (no verbose output)
            auto lex = lexer(filepath, false);
            if (lex.empty()) {
                std::cerr << "Lexer returned no tokens.\n";
                return 1;
            }

            // 2) Parse tokens to build the AST
            Parser parser(lex, false);
            auto ast = parser.parseProgram();

            // 3) Convert the AST to ASDL (assembly intermediate representation)
            auto asdlProgram = convertASTtoASDL(ast);
            if (!asdlProgram) {
                std::cerr << "Failed to convert AST to ASDL.\n";
                return 1;
            }

            // 4) Instead of writing to file, just print the generated assembly code
            std::cout << "\nGenerated ASDL:\n";
            std::cout << asdlProgram->toString() << "\n";

            std::cout << "\nGenerated Assembly:\n";
            std::cout << asdlProgram->toASM() << "\n";

        } else if (mode == "--compile") {
            std::cout << "Full compilation of: " << filepath << "\n";

            // 1. Lexing
            auto lex = lexer(filepath, false);
            if (lex.empty()) {
                std::cerr << "Lexer returned no tokens.\n";
                return 1;
            }

            // 2. Parsing
            Parser parser(lex, false);
            auto ast = parser.parseProgram();

            // 3. AST → ASDL
            auto asdlProgram = convertASTtoASDL(ast);
            if (!asdlProgram) {
                std::cerr << "Failed to convert AST to ASDL.\n";
                return 1;
            }

            // 4. Generate .s file
            const std::string asm_filename = "out.s";
            std::ofstream ofs(asm_filename);
            if (!ofs.is_open()) {
                std::cerr << "Could not write assembly to " << asm_filename << "\n";
                return 1;
            }

            std::string asmCode = asdlProgram->toASM();
            auto posColon = asmCode.find(':');
            if (posColon != std::string::npos) {
                std::string label = asmCode.substr(0, posColon);
                std::string rest = asmCode.substr(posColon + 1);
                ofs << ".globl _" << label << "\n";
                ofs << "_" << label << ":" << rest;
            } else {
                // fallback
                ofs << asmCode;
            }

            ofs.close();

            std::cout << "Assembly written to " << asm_filename << "\n";

            // 5. Determine output executable name from source file
            std::string exec_name = filepath;
            size_t last_slash = exec_name.find_last_of("/\\");
            if (last_slash != std::string::npos)
                exec_name = exec_name.substr(last_slash + 1);

            size_t dot_pos = exec_name.rfind('.');
            if (dot_pos != std::string::npos)
                exec_name = exec_name.substr(0, dot_pos);  // strip extension

            // 6. Assemble and link using clang
            std::string command = "clang -arch x86_64 -o " + exec_name + " out.s";
            int result = system(command.c_str());
            if (result != 0) {
                std::cerr << "Linking failed.\n";
                return 1;
            }

            std::cout << "Compilation succeeded. Executable is 'out'\n";
        } else {
            std::cerr << "Unknown option: " << mode << "\n";
            print_help();
            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
