# Mini C Compiler

A small compiler written in Python that handles a minimal subset of the C language.

## Supported Features

Currently, this compiler can:

- Read a `.c` source file
- Perform lexical and syntactic analysis
- Generate assembly code (x86-64 / ARM depending on the architecture)
- Compile the assembly code into an executable using `gcc`

## Supported C Syntax

The compiler supports **only one type of program**:

```c
int main(void) {
    return 42;
}
