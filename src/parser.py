from ast import Function, Program, ReturnStatement, Constant

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0  # position dans la liste

    def current_token(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def match(self, expected_type):
        token = self.current_token()
        if token and token.type == expected_type:
            self.pos += 1
            return token
        else:
            raise SyntaxError(f"Expected {expected_type}, got {token}")

    def parse_function(self):
        self.match("KEYWORD")         # int
        name_token = self.match("IDENTIFIER")  # main
        self.match("LPAREN")
        self.match("KEYWORD")         # void
        self.match("RPAREN")
        self.match("LBRACE")
        stmt = self.parse_statement()
        self.match("RBRACE")

        return Function(name_token.value, stmt)

    def parse_statement(self):
        self.match("KEYWORD")  # return
        expr = self.parse_expression()
        self.match("SEMICOLON")
        return ReturnStatement(expr)
    
    def parse_expression(self):
        const_token = self.match("CONSTANT")
        return Constant(int(const_token.value))
    
    def parse_program(self):
        func = self.parse_function()

        if self.current_token() is not None:
            extra = self.current_token()
            raise SyntaxError(f"Unexpected token after function: {extra.type} '{extra.value}' at position {extra.position}")

        return Program(func)
