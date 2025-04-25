import re

class LexerError(Exception):
    """Custom exception for lexer errors."""
    def __init__(self, message):
        super().__init__(message)

class Token:
    def __init__(self, type_, value, position=None):
        self.type = type_
        self.value = value
        self.position = position

    def __repr__(self):
        return f"Token(type='{self.type}', value='{self.value}')"

# Define the keywords
KEYWORDS = {"int", "void", "return"}

# Define the regular expressions for tokens
token_specification = [
    ("KEYWORD", r'\b(?:int|void|return)\b'),
    ("IDENTIFIER", r'\b[a-zA-Z_]\w*\b'),
    ("CONSTANT", r'\b[0-9]+\b'),
    ("LPAREN",     r'\('),
    ("RPAREN",     r'\)'),
    ("LBRACE",     r'\{'),
    ("RBRACE",     r'\}'),
    ("SEMICOLON",  r';'),
    ("SKIP",       r'[ \t\n]+'),  # spaces and line breaks
    ("COMMENT",    r'//[^\n]*'),           # single-line comment
    ("ML_COMMENT", r'/\*.*?\*/'),          # multi-line comment (non greedy)
    ("MISMATCH",   r'.'), # any other character → error
]

tok_regex = '|'.join(f'(?P<{name}>{pattern})' for name, pattern in token_specification)
get_token = re.compile(tok_regex).match

def lexer(code):
    pos = 0
    tokens = []
    mo = get_token(code)
    while mo is not None:
        kind = mo.lastgroup
        value = mo.group()
        if kind in ("SKIP", "COMMENT", "ML_COMMENT"):
            pass  # Ignore whitespace and comments
        elif kind == "MISMATCH":
            raise LexerError(f"Unexpected character '{value}' at position {pos}")
        else:
            tokens.append(Token(kind, value, pos))
        pos = mo.end()
        mo = get_token(code, pos)
    return tokens
