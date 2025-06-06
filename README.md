# Mini C Compiler

A small compiler written in c++ that handles a minimal subset of the C language.

## Supported Features

This compiler currently supports:

- Reading a `.c` source file
- Performing lexical and syntactic analysis
- Generating assembly code (x86-64 or ARM, depending on host architecture)
- Assembling and linking the generated assembly using `clang` or `gcc`

## Supported C Syntax

At the moment, the compiler supports only one specific kind of program:

```c
int main(void) {
    return 42;
}
```

## Compilation Output

The compiler generates an assembly file (`.s`) containing the low-level code for the program.

### macOS / Apple Silicon (ARM)

On macOS (especially with Apple Silicon), make sure you target the x86-64 architecture to ensure compatibility with the generated assembly syntax:

```bash
clang -arch x86_64 -o output out.s
```

Then run the program:

```bash
./output
echo $?   # Should print 42
```

### Linux (x86-64)

On Linux with an x86-64 machine, a standard `gcc` or `clang` command should suffice:

```bash
gcc -o output out.s
./output
echo $?   # Should print 42
```

## Notes

- The generated `.s` file uses AT&T syntax.
- The function is labeled `_main` with a `.globl _main` directive.
- Ensure the `.s` file is regenerated and relinked after each modification.

## Example Usage

```bash
./compiler --compile example.c
# This creates out.s and builds an executable named `example` targeting x86_64
```
