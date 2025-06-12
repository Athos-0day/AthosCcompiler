#include "parser.hpp"
#include "ast.hpp"

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

// expect()
Lex Parser::expect(Token expected, const std::string& errorMsg) {
    Lex token = peek();
    if (token.token != expected) {
        error(errorMsg, token);
    }
    return advance();
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
    auto expr = parseExpression(0);
    if (!match(Token::SEMICOLON)) error("Expected ';' after return expression", peek());
    log("Parsed return statement successfully");
    return std::make_unique<ReturnStatement>(std::move(expr));
}

// Get precedence level of a binary operator
int Parser::getPrecedence(Token token) {
    switch (token) {
        case Token::ADDITION:
        case Token::NEGATION: return 45;
        case Token::MULTIPLICATION:
        case Token::DIVISION:
        case Token::REMAINDER: return 50;
        default: return -1;
    }
}

// Convert token to BinaryOpast enum
BinaryOpast Parser::tokenToBinaryOp(Token token) {
    switch (token) {
        case Token::ADDITION: return BinaryOpast::ADD;
        case Token::NEGATION: return BinaryOpast::SUBTRACT;
        case Token::MULTIPLICATION: return BinaryOpast::MULTIPLY;
        case Token::DIVISION:   return BinaryOpast::DIVIDE;
        case Token::REMAINDER: return BinaryOpast::REMAINDER;
        default:
            throw std::runtime_error("Unexpected binary operator token");
    }
}

// parseExpression() with precedence
std::unique_ptr<Expression> Parser::parseExpression(int minPrecedence) {
    log("Parsing expression with precedence >= " + std::to_string(minPrecedence));

    std::unique_ptr<Expression> left = parseFactor();

    while (true) {
        Lex opToken = peek();
        int precedence = getPrecedence(opToken.token);

        if (precedence < minPrecedence) break;

        Token op = opToken.token;
        advance();

        std::unique_ptr<Expression> right = parseExpression(precedence + 1);

        left = std::make_unique<Expression>(tokenToBinaryOp(op), std::move(left), std::move(right));
    }

    return left;
}

// parseFactor()
std::unique_ptr<Expression> Parser::parseFactor() {
    log("Parsing factor");
    Lex currentToken = peek();

    if (currentToken.token == Token::CONSTANT) {
        advance();
        int value = std::stoi(currentToken.word);
        log("Parsed integer literal: " + std::to_string(value));
        return std::make_unique<Expression>(value);
    }
    else if (currentToken.token == Token::COMPLEMENT || currentToken.token == Token::NEGATION) {
        UnaryOpast unop = (currentToken.token == Token::COMPLEMENT) ? UnaryOpast::COMPLEMENT : UnaryOpast::NEGATE;
        advance();
        std::unique_ptr<Expression> operand = parseFactor();
        log("Parsed unary operator: " + currentToken.word);
        return std::make_unique<Expression>(unop, std::move(operand));
    }
    else if (currentToken.token == Token::OPARENTHESIS) {
        advance(); // consume '('
        std::unique_ptr<Expression> inner = parseExpression(0);
        if (peek().token != Token::CPARENTHESIS) {
            throw std::runtime_error("Expected ')' after expression");
        }
        advance(); // consume ')'
        log("Parsed parenthesized expression");
        return inner;
    }
    else {
        throw std::runtime_error("Unexpected token in expression: " + currentToken.word);
    }
}
