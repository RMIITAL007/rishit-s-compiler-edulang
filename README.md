# EduLang Compiler

EduLang is a small educational compiler implemented in C++20. It demonstrates a complete teaching-oriented compiler pipeline:

```text
source -> lexer -> tokens -> parser -> AST -> semantic analysis -> IR -> optimizer -> interpreter / target pseudo-assembly
```

The project is designed for the Compiler Design Laboratory and supports lexical analysis, syntax analysis, AST construction, semantic checking, scoped symbol lookup, intermediate code generation, optimization, interpretation, target-code generation, and diagnostics.

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
docs/            project reports, presentation material, and user documentation
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
1/6 Test #1: compiler_tests ......................   Passed
2/6 Test #2: compiler_symbols_scopes .............   Passed
3/6 Test #3: compiler_benchmark_valid ............   Passed
4/6 Test #4: compiler_benchmark_runtime_error ....   Passed
5/6 Test #5: compiler_benchmark_syntax_error .....   Passed
6/6 Test #6: compiler_benchmark_semantic_error ...   Passed
100% tests passed out of 6
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

Symbol table:

```bash
./build/compiler symbols examples/hello.edu
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

Performance benchmark and optimization profiling:

```bash
./build/compiler bench examples/arithmetic_control.edu
```

The default command prints all pipeline stages and pass/fail statuses:

```bash
./build/compiler examples/hello.edu
```

For an exact, source-located error demonstration, run:

```bash
./build/compiler examples/errors/lexical_error.edu
```

The compiler reports the source file, line, column, source line, and a caret at
the faulty character or expression. It also reports each pipeline stage as
`PASS`, `FAIL`, or `SKIP`.

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

The test source is [tests/test_main.cpp](tests/test_main.cpp). It contains 31
checks for valid programs; lexical, syntax, semantic, and runtime errors; IR
execution; boolean expressions; short-circuiting; checked integer arithmetic;
control flow; nested scopes; variable shadowing; optimization; target-code
generation; and educational hints.

Run the registered test targets with:

```bash
ctest --test-dir build --output-on-failure
```

To display each check individually during the review:

```bash
./build/compiler_tests
```

## Interactive Web Playground & Visualizer

A professional, faculty-facing web workbench built with React, Vite, and TypeScript is included in the `web/` directory. It connects directly to the real C++20 compiler via a sandboxed Node.js adapter.

### Features
- **Monaco-Style Source Editor:** Synchronized line-number gutter, syntax formatting, and pre-loaded sample selector.
- **7 Stage Visualizers:**
  - **Tokens:** Searchable table with coordinate tracking (`line:col`) and token-type categorization.
  - **AST:** Recursive, collapsible $N$-ary tree with *Expand All* / *Collapse All* controls.
  - **Semantic Analysis:** Verification checkpoint cards and formatted diagnostic error carets (`^`) with educational hints (`= hint:`).
  - **Symbol Table:** Scoped table distinguishing active identifiers from out-of-scope shadowed bindings (`x$1`).
  - **Intermediate Code (TAC):** Line-numbered quad listings with jump label badges and copy utility.
  - **Optimization:** Intra-block constant-folding statistics and expression ratio metrics.
  - **Target Code:** EduLang pseudo-assembly viewer with opcode highlighting (`MOV`, `CMPLT`, `JZ`, `PRINT`).
  - **Console Output:** Dedicated runtime stdout box for VM execution results alongside complete raw compiler streams.
- **Subprocess Sandbox Security:** Direct argument passing (no shell) with a strict 2-second timeout protecting against infinite loops.

### Running the Web Playground
```bash
# 1. Navigate to the web application directory
cd web

# 2. Start both the backend (port 5001) and Vite frontend (port 3000)
npm run dev

# 3. Open in your browser:
# http://localhost:3000
```

### Scripted 3-Minute Faculty Demonstration
1. **Full Pipeline Compilation:** Run the default `arithmetic_control.edu` sample. Show that `1 + 2 = 3` is evaluated at compile time, inspect the AST and pseudo-assembly, and show runtime output `6`.
2. **Lexical Variable Shadowing:** Select `symbols_scopes.edu`. Open the **Symbol Table** tab and point out how inner `x` in nested blocks is assigned storage keys `x$1` and `x$2`, preserving outer `x = 1`.
3. **Diagnostic Carets & Educational Hints:** Select `syntax_error.edu` or `semantic_error.edu`. Open the **Semantic Analysis** tab to demonstrate visual caret pointers (`^`) and actionable `= hint:` guidance.
4. **Sandbox Circuit Breaker:** Select `Infinite Loop (Subprocess Timeout Guard)`. Click compile and demonstrate that the backend kills the runaway loop at exactly 2,000ms without freezing the server.

## Phase 3 / Final Review Checklist (100% Complete)


- **Complete Working Pipeline:** Lexer, parser, AST, scoped symbol table, semantic type checker, TAC IR generator, constant-folding optimizer, pseudo-assembly target generator, and runtime interpreter VM.
- **Educational Pinpoint Diagnostics:** Detailed error reporting answering WHERE (line:col + visual carets `^^^^`), WHAT (explicit mistake description), and HOW TO FIX (`= hint:`).
- **Performance Evaluation & Benchmarks:** Built-in high-resolution profiling via `./build/compiler bench <file>` reporting nanosecond/microsecond phase latencies and constant folding reduction percentages.
- **Automated Validation:** 31 passing unit test cases and 6 CMake CTest test targets (100% pass rate).
- **Complete Project Documentation:**
  - [Full 11-Chapter Project Report (Section 24 Format)](docs/final_project_report.md)
  - [User Instructions & Manual](docs/user_guide.md)
  - [Review 3 Presentation & Viva Preparation Guide](docs/final_presentation.md)
  - [Review 3 Presentation Deck (PPTX)](docs/review3.pptx)
  - [Final Report Printable PDF](docs/final_project_report.pdf)

## Known Limitations & Future Enhancements

- Target code is readable pseudo-assembly, not native x86 or ARM machine code.
- The type system currently supports `int` and `bool` (functions, arrays, and strings can be added in future extensions).
- Optimization currently focuses on constant folding across TAC instructions.
