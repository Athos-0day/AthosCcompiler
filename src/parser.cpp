#include "parser.hpp"
#include "ast.hpp"
#include <sstream>
#include <iostream>

// Constructor
Parser::Parser(const std::vector<Lex>& t, bool verboseMode) : tokens(t), verbose(verboseMode) {}

// Error handling
[[noreturn]] void Parser::error(const std::string& message, const Lex& token) {
    std::ostringstream oss;
    oss << "Parse error at line " << token.line << ": " << message;
    throw std::runtime_error(oss.str());
}

// Verbose log
void Parser::log(const std::string& message) {
    if (verbose) {
        std::cout << "[Parser] " << message << std::endl;
    }
}

// Token helpers
Lex Parser::peek() {
    if (current < tokens.size()) {
        return tokens[current];
    } else {
        return Lex{"", Token::MISMATCH, -1};
    }
}

Lex Parser::advance() {
    if (current < tokens.size()) {
        log("Advance token: " + tokens[current].word + " (line " + std::to_string(tokens[current].line) + ")");
        return tokens[current++];
    } else {
        return Lex{"", Token::MISMATCH, -1};
    }
}

bool Parser::match(Token t) {
    if (current < tokens.size() && tokens[current].token == t) {
        log("Match token: " + tokens[current].word + " (line " + std::to_string(tokens[current].line) + ")");
        advance();
        return true;
    }
    return false;
}

Lex Parser::expect(Token expected, const std::string& errorMsg) {
    Lex token = peek();
    if (token.token != expected) {
        error(errorMsg, token);
    }
    return advance();
}

// Parse entry point
std::unique_ptr<Program> Parser::parseProgram() {
    log("Parsing program");
    auto func = parseFunction();
    if (peek().token != Token::MISMATCH) {
        error("Unexpected token after function", peek());
    }
    log("Parsed program successfully");
    return std::make_unique<Program>(std::move(func));
}

// Parse function
std::unique_ptr<Function> Parser::parseFunction() {
    log("Parsing function");

    expect(Token::INT, "Expected 'int' at function start");
    expect(Token::IDENTIFIER, "Expected function name");
    std::string funcName = tokens[current - 1].word;

    expect(Token::OPARENTHESIS, "Expected '(' after function name");
    expect(Token::VOID, "Expected 'void' in parameter list");
    expect(Token::CPARENTHESIS, "Expected ')' after 'void'");

    // Parse function body as a Block
    expect(Token::OBRACE, "Expected '{' to begin function body");
    auto block = parseBlock();  // Now responsible for consuming the closing '}'

    log("Parsed function '" + funcName + "' successfully");
    return std::make_unique<Function>(funcName, std::move(block));
}

//Parse block
std::unique_ptr<Block> Parser::parseBlock() {
    std::vector<std::unique_ptr<BlockItem>> items;

    while (!match(Token::CBRACE)) {
        if (isAtEnd()) {
            throw std::runtime_error("Unexpected end of input. Expected '}' to close block.");
        }

        items.push_back(parseBlockItem());
    }

    return std::make_unique<Block>(std::move(items));
}

// Parse block item (statement or declaration)
std::unique_ptr<BlockItem> Parser::parseBlockItem() {
    if (peek().token == Token::INT) {
        advance(); // consume 'int'
        expect(Token::IDENTIFIER, "Expected identifier after 'int'");
        std::string name = tokens[current - 1].word;

        std::unique_ptr<Expression> init = nullptr;
        if (match(Token::ASSIGN)) {
            init = parseExpression(0);
        }

        expect(Token::SEMICOLON, "Expected ';' after declaration");

        auto decl = std::make_unique<Declaration>(name, std::move(init));
        return std::make_unique<BlockItem>(std::move(decl));
    } else {
        auto stmt = parseStatement();
        return std::make_unique<BlockItem>(std::move(stmt));
    }
}

//Parse ForInit
std::unique_ptr<ForInit> Parser::parseForInit() {
    log("Parsing for-init");

    if (peek().token == Token::INT) {
        // Case: declaration (e.g., int x = 5;)
        advance();  // consume 'int'
        expect(Token::IDENTIFIER, "Expected identifier in for-loop declaration");

        std::string name = tokens[current - 1].word;

        std::unique_ptr<Expression> init = nullptr;
        if (match(Token::ASSIGN)) {
            init = parseExpression(0);
        }

        expect(Token::SEMICOLON, "Expected ';' after for-loop declaration");

        auto decl = std::make_unique<Declaration>(name, std::move(init));
        return std::make_unique<ForInit>(std::move(decl));
    } else {
        // Case: optional expression (e.g., i = 0;)
        std::unique_ptr<Expression> expr = nullptr;

        if (peek().token != Token::SEMICOLON) {
            expr = parseExpression(0);
        }

        expect(Token::SEMICOLON, "Expected ';' after for-loop init expression");

        return std::make_unique<ForInit>(std::move(expr));
    }
}


// Parse statement
std::unique_ptr<Statement> Parser::parseStatement() {
    log("Parsing statement");

    if (match(Token::RETURN)) {
        auto expr = parseExpression(0);
        expect(Token::SEMICOLON, "Expected ';' after return expression");
        log("Parsed return statement");
        return std::make_unique<Statement>(std::move(expr), StatementType::RETURN);
    }

    if (match(Token::BREAK)) {
        expect(Token::SEMICOLON, "Expected ';' after 'break'");
        log("Parsed break statement");
        return Statement::makeBreak();
    }

    if (match(Token::CONTINUE)) {
        expect(Token::SEMICOLON, "Expected ';' after 'continue'");
        log("Parsed continue statement");
        return Statement::makeContinue();
    }

    if (match(Token::IF)) {
        expect(Token::OPARENTHESIS, "Expected '(' after 'if'");
        auto condition = parseExpression(0);
        expect(Token::CPARENTHESIS, "Expected ')' after condition");

        auto thenStmt = parseStatement();
        std::unique_ptr<Statement> elseStmt = nullptr;

        if (match(Token::ELSE)) {
            elseStmt = parseStatement();
        }

        log("Parsed if statement");
        return std::make_unique<Statement>(
            std::move(condition),
            std::move(thenStmt),
            std::move(elseStmt)
        );
    }

    if (match(Token::WHILE)) {
        expect(Token::OPARENTHESIS, "Expected '(' after 'while'");
        auto condition = parseExpression(0);
        expect(Token::CPARENTHESIS, "Expected ')' after while condition");

        auto body = parseStatement();
        log("Parsed while loop");
        return std::make_unique<Statement>(std::move(condition), std::move(body),StatementType::WHILE);
    }

    if (match(Token::DO)) {
        auto body = parseStatement();
        expect(Token::WHILE, "Expected 'while' after 'do' body");
        expect(Token::OPARENTHESIS, "Expected '(' after 'while'");
        auto condition = parseExpression(0);
        expect(Token::CPARENTHESIS, "Expected ')' after condition");
        expect(Token::SEMICOLON, "Expected ';' after do-while loop");

        log("Parsed do-while loop");
        return std::make_unique<Statement>(std::move(condition),std::move(body),StatementType::DO_WHILE);
    }

    if (match(Token::FOR)) {
        expect(Token::OPARENTHESIS, "Expected '(' after 'for'");
        auto init = parseForInit();

        std::unique_ptr<Expression> condition = nullptr;
        if (!match(Token::SEMICOLON)) {
            condition = parseExpression(0);
            expect(Token::SEMICOLON, "Expected ';' after for-loop condition");
        }

        std::unique_ptr<Expression> post = nullptr;
        if (peek().token != Token::CPARENTHESIS) {
            post = parseExpression(0);
        }

        expect(Token::CPARENTHESIS, "Expected ')' after for-loop clauses");
        auto body = parseStatement();

        log("Parsed for loop");
        return std::make_unique<Statement>(
            std::move(init),
            std::move(condition),
            std::move(post),
            std::move(body)
        );
    }

    if (match(Token::OBRACE)) {
        auto block = parseBlock();
        log("Parsed compound block statement");
        return std::make_unique<Statement>(std::move(block));
    }

    if (peek().token == Token::SEMICOLON) {
        advance();
        log("Parsed empty statement");
        return std::make_unique<Statement>(nullptr, StatementType::NULL_STMT);
    }

    auto expr = parseExpression(0);
    expect(Token::SEMICOLON, "Expected ';' after expression statement");
    log("Parsed expression statement");
    return std::make_unique<Statement>(std::move(expr), StatementType::EXPRESSION);
}


// Get precedence level of binary operators
int Parser::getPrecedence(Token token) {
    switch (token) {
        case Token::ADDITION:
        case Token::NEGATION: return 45;
        case Token::MULTIPLICATION:
        case Token::DIVISION:
        case Token::REMAINDER: return 50;
        case Token::LESS:
        case Token::LESSEQ:
        case Token::GREATER:
        case Token::GREATEREQ: return 35;
        case Token::EQUAL:
        case Token::NOTEQUAL: return 30;
        case Token::AND: return 10;
        case Token::OR: return 5;
        case Token::QUESTION_MARK: return 3;
        case Token::ASSIGN: return 1;
        default: return -1;
    }
}

// Convert token to BinaryOpast enum
BinaryOpast Parser::tokenToBinaryOp(Token token) {
    switch (token) {
        case Token::ADDITION: return BinaryOpast::ADD;
        case Token::NEGATION: return BinaryOpast::SUBTRACT;
        case Token::MULTIPLICATION: return BinaryOpast::MULTIPLY;
        case Token::DIVISION: return BinaryOpast::DIVIDE;
        case Token::REMAINDER: return BinaryOpast::REMAINDER;
        case Token::AND: return BinaryOpast::AND;
        case Token::OR: return BinaryOpast::OR;
        case Token::EQUAL: return BinaryOpast::EQUAL;
        case Token::NOTEQUAL: return BinaryOpast::NOTEQUAL;
        case Token::LESS: return BinaryOpast::LESSTHAN;
        case Token::LESSEQ: return BinaryOpast::LESSEQ;
        case Token::GREATER: return BinaryOpast::GREATERTHAN;
        case Token::GREATEREQ: return BinaryOpast::GREATEREQ;
        default:
            throw std::runtime_error("Unexpected binary operator token");
    }
}

// Convert token to UnaryOpast
UnaryOpast Parser::tokenToUnaryOp(Token token) {
    switch (token) {
        case Token::COMPLEMENT: return UnaryOpast::COMPLEMENT;
        case Token::NEGATION: return UnaryOpast::NEGATE;
        case Token::NOT: return UnaryOpast::NOT;
        default:
            throw std::runtime_error("Unexpected unary operator token");
    }
}

std::unique_ptr<Expression> Parser::parseExpression(int minPrecedence) {
    log("Parsing expression with precedence >= " + std::to_string(minPrecedence));

    std::unique_ptr<Expression> left = parseFactor();

    while (true) {
        Lex opToken = peek();
        Token op = opToken.token;

        // Special handling for conditional operator (ternary ?:)
        if (op == Token::QUESTION_MARK) {
            if (minPrecedence > getPrecedence(Token::QUESTION_MARK)) {
                break;
            }

            advance();  // consume '?'
            log("Parsing true branch of conditional expression");

            std::unique_ptr<Expression> trueExpr = parseExpression(1); // higher than ?:

            if (peek().token != Token::COLON) {
                error("Expected ':' in conditional expression", peek());
            }

            advance();  // consume ':'
            log("Parsing false branch of conditional expression");

            std::unique_ptr<Expression> falseExpr = parseExpression(1); // same level as trueExpr

            if (!left || !trueExpr || !falseExpr) {
                error("Incomplete conditional expression", opToken);
            }

            left = std::make_unique<Expression>(std::move(left), std::move(trueExpr), std::move(falseExpr));
            log("Parsed conditional expression");
            continue;
        }

        int precedence = getPrecedence(op);
        if (precedence < minPrecedence) break;

        advance();  // consume operator

        if (op == Token::ASSIGN) {
            std::unique_ptr<Expression> right = parseExpression(precedence);
            left = std::make_unique<Expression>(std::move(left), std::move(right));
        } else {
            std::unique_ptr<Expression> right = parseExpression(precedence + 1);
            left = std::make_unique<Expression>(tokenToBinaryOp(op), std::move(left), std::move(right));
        }
    }

    return left;
}

// Parse factor
std::unique_ptr<Expression> Parser::parseFactor() {
    log("Parsing factor");
    Lex currentToken = peek();

    if (currentToken.token == Token::CONSTANT) {
        advance();
        int value = std::stoi(currentToken.word);
        log("Parsed integer literal: " + std::to_string(value));
        return std::make_unique<Expression>(value);
    }
    else if (currentToken.token == Token::IDENTIFIER) {
        advance();
        log("Parsed identifier: " + currentToken.word);
        return std::make_unique<Expression>(currentToken.word);
    }
    else if (currentToken.token == Token::COMPLEMENT ||
             currentToken.token == Token::NEGATION ||
             currentToken.token == Token::NOT) {

        UnaryOpast unop = tokenToUnaryOp(currentToken.token);
        advance();
        std::unique_ptr<Expression> operand = parseFactor();
        log("Parsed unary operator: " + currentToken.word);
        return std::make_unique<Expression>(unop, std::move(operand));
    }
    else if (currentToken.token == Token::OPARENTHESIS) {
        advance();
        std::unique_ptr<Expression> inner = parseExpression(0);
        expect(Token::CPARENTHESIS, "Expected ')' after expression");
        log("Parsed parenthesized expression");
        return inner;
    }
    else {
        error("Unexpected token in expression: " + currentToken.word, currentToken);
        return nullptr;
    }
}
