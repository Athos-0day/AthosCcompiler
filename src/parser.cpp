#include "parser.hpp"

// Constructor
Parser::Parser(const std::vector<Lex>& t, bool verboseMode) : tokens(t), verbose(verboseMode) {}

// error()
[[noreturn]] void Parser::error(const std::string& message, const Lex& token) {
    std::ostringstream oss;
    oss << "Parse error at line " << token.line << ": " << message;
    throw std::runtime_error(oss.str());
}

// log()
void Parser::log(const std::string& message) {
    if (verbose) {
        std::cout << "[Parser] " << message << std::endl;
    }
}

// peek()
Lex Parser::peek() {
    if (current < tokens.size()) {
        return tokens[current];
    } else {
        return Lex{"", Token::MISMATCH, -1};
    }
}

// advance()
Lex Parser::advance() {
    if (current < tokens.size()) {
        log("Advance token: " + tokens[current].word + " (line " + std::to_string(tokens[current].line) + ")");
        return tokens[current++];
    } else {
        return Lex{"", Token::MISMATCH, -1};
    }
}

// match()
bool Parser::match(Token t) {
    if (current < tokens.size() && tokens[current].token == t) {
        log("Match token: " + tokens[current].word + " (line " + std::to_string(tokens[current].line) + ")");
        advance();
        return true;
    }
    return false;
}

// parseProgram()
std::unique_ptr<Program> Parser::parseProgram() {
    log("Parsing program");
    auto func = parseFunction();
    if (peek().token != Token::MISMATCH) {
        error("Unexpected token after function", peek());
    }
    log("Parsed program successfully");
    return std::make_unique<Program>(std::move(func));
}


// parseFunction()
std::unique_ptr<Function> Parser::parseFunction() {
    log("Parsing function");
    if (!match(Token::INT)) error("Expected 'int' at the beginning of function", peek());
    if (!match(Token::IDENTIFIER)) error("Expected function name (identifier)", peek());
    std::string funcName = tokens[current - 1].word;
    if (!match(Token::OPARENTHESIS)) error("Expected '(' after function name", peek());
    if (!match(Token::VOID)) error("Expected 'void' in function parameters", peek());
    if (!match(Token::CPARENTHESIS)) error("Expected ')' after 'void'", peek());
    if (!match(Token::OBRACE)) error("Expected '{' to start function body", peek());
    auto stmt = parseStatement();
    if (!match(Token::CBRACE)) error("Expected '}' to close function body", peek());
    log("Parsed function '" + funcName + "' successfully");
    return std::make_unique<Function>(funcName, std::move(stmt));
}

// parseStatement()
std::unique_ptr<ReturnStatement> Parser::parseStatement() {
    log("Parsing statement");
    if (!match(Token::RETURN)) error("Expected 'return' at beginning of statement", peek());
    auto expr = parseExpression();
    if (!match(Token::SEMICOLON)) error("Expected ';' after return expression", peek());
    log("Parsed return statement successfully");
    return std::make_unique<ReturnStatement>(std::move(expr));
}

// parseExpression()
std::unique_ptr<Expression> Parser::parseExpression() {
    log("Parsing expression");
    Lex currentToken = peek();
    if (currentToken.token != Token::CONSTANT) {
        error("Expected integer constant in expression", currentToken);
    }
    advance();
    int value = std::stoi(currentToken.word);
    log("Parsed expression with value " + std::to_string(value));
    return std::make_unique<Expression>(value);
}
