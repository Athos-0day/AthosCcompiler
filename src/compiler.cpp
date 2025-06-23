#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "asdl.hpp"
#include "tacky.hpp"
#include "lowerer.hpp"
#include "validate.hpp"


void print_help() {
    std::cout << "Usage:\n";
    std::cout << "  ./compiler --lex <source_file>      # Run the lexer only\n";
    std::cout << "  ./compiler --parse <source_file>    # Run the lexer and parser with verbose logs\n";
    std::cout << "  ./compiler --validate <source_file> # Run semantic validation\n";
    std::cout << "  ./compiler --tacky <source_file>    # Lower to TACKY intermediate code and print\n";
    std::cout << "  ./compiler --codegen <source_file>  # Generate assembly from parsed AST\n";
    std::cout << "  ./compiler <source_file>            # Compile and link (default behavior)\n";
    std::cout << "  ./compiler --help                   # Show this help message\n";
}

int main(int argc, char* argv[]) {
    std::string filepath;
    std::string mode;

    if (argc == 2) {
        if (std::string(argv[1]) == "--help") {
            print_help();
            return 0;
        }
        mode = "--compile";
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
            auto lex = lexer(filepath, /*verbose=*/true);
            if (lex.empty()) {
                std::cerr << "Lexer returned no tokens.\n";
                return 1;
            }

        } else if (mode == "--parse") {
            std::cout << "Running lexer and parser on: " << filepath << "\n";
            auto lex = lexer(filepath, /*verbose=*/false);
            Parser parser(lex, /*verbose=*/true);
            parser.parseProgram();
            std::cout << "Parsing completed successfully.\n";

        } else if (mode == "--validate") {
            std::cout << "Running semantic validation on: " << filepath << "\n";

            auto lex = lexer(filepath, /*verbose=*/false);
            Parser parser(lex, /*verbose=*/false);
            auto program = parser.parseProgram();

            validate_verbose = true;

            resolve_program(program.get());

            std::cout << "Semantic validation completed successfully.\n";
        } else if (mode == "--tacky") {
            std::cout << "Lowering AST to TACKY for: " << filepath << "\n";
            auto lex = lexer(filepath, false);
            Parser parser(lex, false);
            auto ast = parser.parseProgram();
            Lowerer lowerer;
            auto tackyProgram = lowerer.lower(ast.get());

            std::cout << "\nGenerated TACKY IR:\n";
            std::cout << tackyProgram->toString() << "\n";

        } else if (mode == "--codegen") {
            std::cout << "Generating assembly from: " << filepath << "\n";
            auto lex = lexer(filepath, false);
            Parser parser(lex, false);
            auto ast = parser.parseProgram();

            Lowerer lowerer;
            auto tackyProgram = lowerer.lower(ast.get());

            ASDLProgram asdlProgram = convertTackyToASDL(*tackyProgram);
            int stackOffset = replacePseudosWithStack(asdlProgram);  
            insertAllocateStack(asdlProgram,-stackOffset);
            legalizeMovMemoryToMemory(asdlProgram);

            std::cout << "\nGenerated ASDL:\n";
            std::cout << asdlProgram.toString() << "\n";

            std::cout << "\nGenerated Assembly:\n";
            std::cout << asdlProgram.toASM() << "\n";

            std::cout << "stackoffset value = " << stackOffset << std::endl;

        } else if (mode == "--compile") {
            std::cout << "Full compilation of: " << filepath << "\n";
            auto lex = lexer(filepath, false);
            Parser parser(lex, false);
            auto ast = parser.parseProgram();

            Lowerer lowerer;
            auto tackyProgram = lowerer.lower(ast.get());

            ASDLProgram asdlProgram = convertTackyToASDL(*tackyProgram);
            int stackOffset = replacePseudosWithStack(asdlProgram);
            insertAllocateStack(asdlProgram, -stackOffset); 
            legalizeMovMemoryToMemory(asdlProgram);

            const std::string asm_filename = "out.s";
            try {
                writeASMToFile(asdlProgram, asm_filename);
            } catch (const std::exception& e) {
                std::cerr << "Error while writing assembly: " << e.what() << "\n";
                return 1;
            }

            std::string exec_name = filepath;
            size_t last_slash = exec_name.find_last_of("/\\");
            if (last_slash != std::string::npos)
                exec_name = exec_name.substr(last_slash + 1);
            size_t dot_pos = exec_name.rfind('.');
            if (dot_pos != std::string::npos)
                exec_name = exec_name.substr(0, dot_pos);

            std::string command = "clang -arch x86_64 -o " + exec_name + " out.s";
            int result = system(command.c_str());
            if (result != 0) {
                std::cerr << "Linking failed.\n";
                return 1;
            }

            std::cout << "Compilation succeeded. Executable is '" << exec_name << "'\n";
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
