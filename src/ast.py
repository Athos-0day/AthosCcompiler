class ASTNode:
    def __repr__(self):
        return self.__str__()

class Program(ASTNode):
    def __init__(self, function):
        self.function = function

    def __str__(self):
        # "Program" node, no prefix, and add indentation for function
        return f"Program\n{indent("└── " +str(self.function), level=0)}"

class Function(ASTNode):
    def __init__(self, name, body):
        self.name = name
        self.body = body

    def __str__(self):
        # Function node with a "└── " and indent the body
        return f"Function: {self.name}\n{indent("└── " +str(self.body), level=1)}"

class ReturnStatement(ASTNode):
    def __init__(self, expression):
        self.expression = expression

    def __str__(self):
        # Return statement node, and indent the expression
        return f"Return\n{indent("└── " +str(self.expression), level=2)}"

class Constant(ASTNode):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        # Constant value
        return f"Constant: {self.value}"

def indent(text, level=1):
    """Indent each line of text by adding the given level of indentation with the prefix "└── "."""
    prefix = "    " * level  # 4 spaces per level of indentation
    return "\n".join(prefix + line for line in text.splitlines())
