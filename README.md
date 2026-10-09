# EduLang Compiler

EduLang is a small educational compiler implemented in C++20. It demonstrates a complete teaching-oriented compiler pipeline:

```text
source -> lexer -> tokens -> parser -> AST -> semantic analysis -> IR -> optimizer -> interpreter / target pseudo-assembly
```

The project is designed for Compiler Design Review 2 and supports lexical analysis, syntax analysis, AST construction, semantic checking, scoped symbol lookup, intermediate code generation, optimization, interpretation, target-code generation, and diagnostics.

## Language Features

EduLang supports:

- integer declarations and assignments
- boolean literals: `true` and `false`
- arithmetic operators: `+`, `-`, `*`, `/`
- comparison operators: `<`, `<=`, `>`, `>=`
- equality operators: `==`, `!=`
- logical operators: `&&`, `||`, `!`
- `print(expression);`
- `if`, `else`, and `while`
- brace-delimited nested blocks
- line comments beginning with `//`
- block comments enclosed by `/*` and `*/`

A short grammar summary is:

```text
program       -> statement* EOF
statement     -> declaration | assignment | print | if | while | block
block         -> "{" statement* "}"
declaration   -> "int" identifier "=" expression ";"
assignment    -> identifier "=" expression ";"
print         -> "print" "(" expression ")" ";"
if            -> "if" "(" expression ")" statement ("else" statement)?
while         -> "while" "(" expression ")" statement
expression    -> logical-or
```

## Folder Structure

```text
include/edulang/
  ast/          AST types and AST printer interface
  common/       source positions and spans
  frontend/     lexer, parser, diagnostics, semantic analyzer, symbol table
  ir/           intermediate representation, generator, optimizer
  runtime/      interpreter interface
  target/       target pseudo-assembly interface
src/
  ast/          AST printer implementation
  frontend/     frontend implementations
  ir/           IR printer, generator, optimizer
  runtime/      interpreter implementation
  target/       target pseudo-assembly generator
tests/           compiler test executable
examples/        valid and invalid EduLang programs
docs/            Phase 1 and Review 2 documentation
build/           generated CMake output, ignored by Git
CMakeLists.txt   CMake build and CTest configuration
```

## Build

Run these commands from the project root:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The verified result is:

```text
1/1 Test #1: compiler_tests ... Passed
100% tests passed out of 1
```

## Running Compiler Stages

Tokens:

```bash
./build/compiler tokens examples/hello.edu
```

AST:

```bash
./build/compiler ast examples/hello.edu
```

Intermediate code:

```bash
./build/compiler ir examples/hello.edu
```

Execution:

```bash
./build/compiler run examples/hello.edu
```

Target pseudo-assembly:

```bash
./build/compiler target examples/hello.edu
```

The default command prints all available stages:

```bash
./build/compiler examples/hello.edu
```

## Sample Input and Output

Input:

```text
int a = 10;
int b = 20;
print(a + b);
```

Command:

```bash
./build/compiler run examples/hello.edu
```

Verified output:

```text
Output
30
```

A control-flow example was also executed successfully:

```text
int a = 0;
while (a < 3) { print(a); a = a + 1; }
if (a == 3) { print(true); } else { print(false); }
```

Verified output:

```text
Output
0
1
2
true
```

The target stage produces readable pseudo-assembly, for example:

```text
Target Code
; EduLang pseudo-assembly
MOV t0, #10
MOV [a], t0
MOV t1, #20
MOV [b], t1
MOV t2, [a]
MOV t3, [b]
ADD t4, t2, t3
PRINT t4
```

## Tests

The test source is [tests/test_main.cpp](tests/test_main.cpp). It covers valid programs, lexical errors, syntax errors, semantic errors, runtime errors, IR execution, boolean expressions, control flow, nested scopes, optimization, target-code generation, and edge cases.

Run the registered test target with:

```bash
ctest --test-dir build --output-on-failure
```

## Known Limitations

- Target code is readable pseudo-assembly, not native x86 or ARM machine code.
- The type system currently supports only `int` and `bool`.
- There are no functions, strings, arrays, or user-defined types.
- Optimization currently focuses on constant folding.
- The project has one CTest executable containing the test cases.
