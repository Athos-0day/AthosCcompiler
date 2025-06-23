/**
 * @file lexer.cpp
 * @brief A simple lexer for a subset of C-like language constructs.
 * 
 * This lexer reads a source file, tokenizes it into a series of tokens such as
 * identifiers, constants, keywords, punctuation, and comments. It skips whitespace
 * and comments by default. The lexer throws exceptions on encountering invalid tokens.
 * 
 * Supported tokens:
 * - Keywords: int, void, return
 * - Identifiers: C-style variable/function names
 * - Numeric constants (integers)
 * - Single-line comments (//...)
 * - Multi-line comments 
 * - Punctuation: parentheses, braces, semicolon
 * - Unary Operator: complement, negation, decrement
 * - Binary Operator: addition, division, multiplication, remainder
 * - Logical and relational operators: and, or, equal, not equal, greater, less, greater or equal, less or equal
 * - Assignment for local variable
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <stdexcept>
#include <algorithm> 

/**
 * @enum Token
 * @brief Enumeration of token types recognized by the lexer.
 */
enum class Token {
    IDENTIFIER,     ///< Valid identifier (e.g., variable names)
    CONSTANT,       ///< Numeric constant (digits only)
    INT,            ///< 'int' keyword
    VOID,           ///< 'void' keyword
    RETURN,         ///< 'return' keyword
    OPARENTHESIS,   ///< Opening parenthesis '('
    CPARENTHESIS,   ///< Closing parenthesis ')'
    OBRACE,         ///< Opening brace '{'
    CBRACE,         ///< Closing brace '}'
    SEMICOLON,      ///< Semicolon ';'
    SKIP,           ///< Whitespace characters (spaces, tabs, newlines)
    COMMENT,        ///< Single-line comment (//...)
    ML_COMMENT,     ///< Multi-line comment (/* ... */)
    MISMATCH,       ///< Invalid or unrecognized token
    COMPLEMENT,     ///< Complement operator '~'
    NEGATION,       ///< Negation operator '-'
    DECREMENT,      ///< Decrement operator '--'
    ADDITION,       ///< Addition operator '+'
    MULTIPLICATION, ///< Multiplication operator '*'
    DIVISION,       ///< Division operator '/'
    REMAINDER,      ///< Remainder operator '%'
    NOT,            ///< Not Operator '!' 
    AND,            ///< And Operator '&&'
    OR,             ///< Or Operator '||'
    EQUAL,          ///< Equal Operator '=='
    NOTEQUAL,       ///< Not Equal Operator '!='
    LESS,           ///< Less '<'
    GREATER,        ///< Greater '>'
    LESSEQ,         ///< Less or Equal '<='
    GREATEREQ,      ///< Greater or Equal '>='
    ASSIGN          ///< Assignement '='
};

/**
 * @struct Lex
 * @brief Represents a lexical token.
 * 
 * Contains the raw token string, the token type, its position (order) in the input stream,
 * and the line number where the token appears in the source file.
 */
struct Lex {
    std::string word;   ///< Raw token text
    Token token;        ///< Token type
    int position;       ///< Position index of token in the source
    int line;           ///< Line number where the token was found
};

/**
 * @brief Converts a Token enum value to a human-readable string.
 * 
 * @param t The token type to convert.
 * @return std::string A string representing the token type.
 */
std::string tokenToString(Token t) {
    switch (t) {
        case Token::IDENTIFIER: return "IDENTIFIER";
        case Token::CONSTANT: return "CONSTANT";
        case Token::INT: return "INT";
        case Token::VOID: return "VOID";
        case Token::RETURN: return "RETURN";
        case Token::OPARENTHESIS: return "OPARENTHESIS";
        case Token::CPARENTHESIS: return "CPARENTHESIS";
        case Token::OBRACE: return "OBRACE";
        case Token::CBRACE: return "CBRACE";
        case Token::SEMICOLON: return "SEMICOLON";
        case Token::SKIP: return "SKIP";
        case Token::COMMENT: return "COMMENT";
        case Token::ML_COMMENT: return "ML_COMMENT";
        case Token::COMPLEMENT: return "COMPLEMENT";
        case Token::NEGATION: return "NEGATION";
        case Token::DECREMENT: return "DECREMENT";
        case Token::ADDITION: return "ADDITION";
        case Token::MULTIPLICATION: return "MULTIPLICATION";
        case Token::DIVISION: return "DIVISION";
        case Token::REMAINDER: return "REMAINDER";
        case Token::NOT: return "NOT";
        case Token::AND: return "AND";
        case Token::OR: return "OR";
        case Token::EQUAL: return "EQUAL";
        case Token::NOTEQUAL: return "NOT EQUAL";
        case Token::LESS: return "LESS";
        case Token::GREATER: return "GREATER";
        case Token::LESSEQ: return "LESS OR EQUAL";
        case Token::GREATEREQ: return "GREATER OR EQUAL";
        case Token::ASSIGN: return "ASSIGN";
        default: return "MISMATCH";
    }
}

/**
 * @brief Determines the token type for a given word string.
 * 
 * Matches keywords, punctuation, identifiers, constants, comments, and whitespace using regex.
 * Returns Token::MISMATCH if the input does not match any known token pattern.
 * 
 * @param word The input string to classify.
 * @return Token The corresponding token type.
 */
Token wordToToken(const std::string& word) {
    if (word == "int")      return Token::INT;
    if (word == "void")     return Token::VOID;
    if (word == "return")   return Token::RETURN;
    if (word == "(")        return Token::OPARENTHESIS;
    if (word == ")")        return Token::CPARENTHESIS;
    if (word == "{")        return Token::OBRACE;
    if (word == "}")        return Token::CBRACE;
    if (word == ";")        return Token::SEMICOLON;
    if (word == "~")        return Token::COMPLEMENT;
    if (word == "--")       return Token::DECREMENT;
    if (word == "-")        return Token::NEGATION;
    if (word == "+")        return Token::ADDITION;
    if (word == "*")        return Token::MULTIPLICATION;
    if (word == "/")        return Token::DIVISION;
    if (word == "%")        return Token::REMAINDER;
    if (word == "&&")       return Token::AND;
    if (word == "||")       return Token::OR;
    if (word == "==")       return Token::EQUAL;
    if (word == "!=")       return Token::NOTEQUAL;
    if (word == "!")        return Token::NOT;
    if (word == "<")        return Token::LESS;
    if (word == ">")        return Token::GREATER;
    if (word == "<=")       return Token::LESSEQ;
    if (word == ">=")       return Token::GREATEREQ;
    if (word == "=")        return Token::ASSIGN;

    if (std::regex_match(word, std::regex(R"(\d+)"))) 
        return Token::CONSTANT;

    if (std::regex_match(word, std::regex(R"([a-zA-Z_]\w*)"))) 
        return Token::IDENTIFIER;

    if (std::regex_match(word, std::regex(R"(^\s+$)"))) 
        return Token::SKIP;

    if (std::regex_match(word, std::regex(R"(//.*)"))) 
        return Token::COMMENT;

    if (std::regex_match(word, std::regex(R"(/\*.*\*/)"))) 
        return Token::ML_COMMENT;

    return Token::MISMATCH;
}

/**
 * @brief Performs lexical analysis on the given source file.
 * 
 * Reads the entire file contents, then repeatedly matches and extracts tokens
 * according to predefined regular expression patterns. Skips whitespace and comments.
 * Throws an exception if an invalid token is encountered.
 * 
 * The recognized tokens include identifiers, constants, keywords (int, void, return),
 * punctuation (parentheses, braces, semicolon), comments (single-line and multi-line),
 * and whitespace.
 * 
 * This function also tracks the line number for each token and stores it in the Lex struct,
 * so that errors or debug info can reference the exact line.
 * 
 * @param filename The path to the source file to lex.
 * @param verbose If true, prints all tokens with their types, positions, and lines.
 * @return std::vector<Lex> A vector of Lex objects representing the tokens found.
 * 
 * @throws std::runtime_error If the file cannot be opened or if a lexical error occurs.
 */
std::vector<Lex> lexer(const std::string& filename, bool verbose = true) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }

    // Read entire file content into a string
    std::string input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Normalize newlines (remove '\r' if present)
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());

    std::vector<Lex> lexemes;  // Vector to hold tokens
    int position = 0;          // Position index of tokens
    int lineNumber = 1;        // Current line number in source code

    // Patterns for different token types in priority order
    std::vector<std::pair<std::regex, Token>> patterns = {
        {std::regex(R"(^#[^\n]*)"), Token::SKIP},
        {std::regex(R"(^//[^\n]*)"), Token::COMMENT},
        {std::regex(R"(^/\*([^*]|\*+[^*/])*\*+/)"), Token::ML_COMMENT},
        {std::regex(R"(^\()"), Token::OPARENTHESIS},
        {std::regex(R"(^\))"), Token::CPARENTHESIS},
        {std::regex(R"(^\{)"), Token::OBRACE},
        {std::regex(R"(^\})"), Token::CBRACE},
        {std::regex(R"(^;)"), Token::SEMICOLON},
        {std::regex(R"(^~)"), Token::COMPLEMENT},
        {std::regex(R"(^--)"), Token::DECREMENT},
        {std::regex(R"(^-)"), Token::NEGATION},
        {std::regex(R"(^\+)"), Token::ADDITION},
        {std::regex(R"(^\*)"), Token::MULTIPLICATION},
        {std::regex(R"(^\/)"), Token::DIVISION},
        {std::regex(R"(^\%)"), Token::REMAINDER},
        {std::regex(R"(^&&)"), Token::AND},
        {std::regex(R"(^\|\|)"), Token::OR},
        {std::regex(R"(^==)"), Token::EQUAL},
        {std::regex(R"(^!=)"), Token::NOTEQUAL},
        {std::regex(R"(^!)"), Token::NOT},
        {std::regex(R"(^<=)"), Token::LESSEQ},
        {std::regex(R"(^>=)"), Token::GREATEREQ},
        {std::regex(R"(^<)"), Token::LESS},
        {std::regex(R"(^>)"), Token::GREATER},
        {std::regex(R"(^\d+[a-zA-Z_]\w*)"), Token::MISMATCH}, // invalid token like 123abc
        {std::regex(R"(^[a-zA-Z_]\w*)"), Token::IDENTIFIER},
        {std::regex(R"(^\d+)"), Token::CONSTANT},
        {std::regex(R"(^\s+)"), Token::SKIP},
        {std::regex(R"(^=)"), Token::ASSIGN},
        {std::regex(R"(^\S)"), Token::MISMATCH},
    };

    // Process input until fully consumed
    while (!input.empty()) {
        std::smatch match;
        bool matched = false;

        // Try each pattern in order to find a match at start of input
        for (const auto& [regex, tokenType] : patterns) {
            if (std::regex_search(input, match, regex)) {
                std::string token = match.str(0);

                // Skip whitespace and comments without adding tokens, but update line count
                if (tokenType == Token::SKIP || tokenType == Token::COMMENT || tokenType == Token::ML_COMMENT) {
                    // Count newlines in skipped text to keep line count accurate
                    lineNumber += std::count(token.begin(), token.end(), '\n');
                    input.erase(0, token.length());
                    matched = true;
                    break;
                }

                // Determine actual token for identifiers, constants, or mismatches
                Token actualToken = tokenType;
                if (tokenType == Token::IDENTIFIER || tokenType == Token::CONSTANT || tokenType == Token::MISMATCH) {
                    actualToken = wordToToken(token);
                }

                if (actualToken == Token::MISMATCH) {
                    std::ostringstream oss;
                    oss << "Lexical error: invalid token '" << token
                        << "' at line " << lineNumber
                        << ", position " << position;
                    throw std::runtime_error(oss.str());
                }

                // Count newlines in the matched token to update line number
                int newlines = std::count(token.begin(), token.end(), '\n');

                // Store the current line number for this token
                int tokenLine = lineNumber;

                // Update lineNumber for the next token
                lineNumber += newlines;

                // Add the token with line and position info
                lexemes.push_back({token, actualToken, position++, tokenLine});

                // Remove matched token from input
                input.erase(0, token.length());
                matched = true;
                break;
            }
        }

        // If no pattern matched, this is a lexical error
        if (!matched) {
            std::ostringstream oss;
            oss << "Lexical error: could not match input at line "
                << lineNumber << ", position " << position;
            throw std::runtime_error(oss.str());
        }
    }

    // If verbose, print all tokens with their word, type, position, and line number
    if (verbose) {
        for (const auto& lex : lexemes) {
            std::cout << "Token: \"" << lex.word
                      << "\", Type: " << tokenToString(lex.token)
                      << ", Position: " << lex.position
                      << ", Line: " << lex.line << std::endl;
        }
    }

    return lexemes;
}
