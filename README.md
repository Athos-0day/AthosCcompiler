# Mini C Compiler

A small compiler written in C++ that handles a minimal subset of the C language.

## Supported Features

This compiler currently supports:

- Reading a `.c` source file
- Performing lexical and syntactic analysis
- Generating assembly code (x86-64 or ARM, depending on host architecture)
- Assembling and linking the generated assembly using `clang` or `gcc`

## Supported C Syntax

At the moment, the compiler supports simple programs including:

```c
int main(void) {
    return 42;
}
```

In addition, the compiler supports:

### Control Flow:
- `if`, `else if`, and `else` statements  
  ```c
  int main(void) {
      int x = 1;
      if (x)
          return 10;
      else
          return 20;
  }
  ```

- Conditional expressions (ternary operator)  
  ```c
  int main(void) {
      int x = 5;
      return (x > 3) ? 1 : 0;
  }
  ```

- Compound statements (block scope with `{}`)  
  ```c
  int main(void) {
      int x = 5;
      if (x > 0) {
          int y = 3;
          x = x + y;
      }
      return x;
  }
  ```

- Loops: `for`, `while`, and `do-while`
  ```c
  int main(void) {
      int i = 0;
      while (i < 10) {
          if (i == 5)
              break;
          i = i + 1;
      }

      for (int j = 0; j < 10; j = j + 1) {
          if (j % 2 == 0)
              continue;
          i = i + j;
      }

      do {
          i = i - 1;
      } while (i > 0);

      return i;
  }
  ```

- Loop Control Statements:
  - `break` to exit early from loops
  - `continue` to skip the current iteration and proceed to the next

### Unary Operators:
- `-x` (negation)
- `~x` (bitwise NOT)
- `!x` (logical NOT)

### Binary Operators:

#### Arithmetic:
- `+` (addition)
- `-` (subtraction)
- `*` (multiplication)
- `/` (division)
- `%` (modulo)

#### Logical and Comparison Operators:
- `<` (less than)
- `<=` (less or equal)
- `>` (greater than)
- `>=` (greater or equal)
- `==` (equal)
- `!=` (not equal)
- `&&` (logical AND)
- `||` (logical OR)

These can be used inside expressions including returns, assignments, or conditions:

```c
int main(void) {
    return !(5 + 3 < 10) && (2 == 2 || 0);
}
```

### Local Variables

The compiler supports local variables of type `int`:

```c
int main(void) {
    int a = 10;
    int b = 20;
    int c = a + b;
    return c;
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
echo $?   # Should print the result of the return expression
```

### Linux (x86-64)

On Linux with an x86-64 machine, a standard `gcc` or `clang` command should suffice:

```bash
gcc -o output out.s
./output
echo $?   # Should print the return value
```

## Notes

- The generated `.s` file uses AT&T syntax.
- The function is labeled `_main` with a `.globl _main` directive.
- Ensure the `.s` file is regenerated and relinked after each modification.

## Example Usage

```bash
./compiler example.c
# This creates out.s and builds an executable named `example` targeting x86_64
```
