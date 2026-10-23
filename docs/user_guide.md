# EduLang Compiler: User Instructions & Demonstration Manual

This document provides step-by-step instructions for building, testing, inspecting compiler stages, running programs, benchmarking performance, and demonstrating educational error diagnoses.

---

## 1. Prerequisites & Toolchain
- **Operating System:** macOS, Linux, or Windows (WSL / MinGW)
- **Compiler:** Clang++ (Apple Clang 14+ or LLVM Clang 14+) or GCC 11+ with C++20 support
- **Build System:** CMake 3.20 or newer
- **Make/Ninja:** Standard UNIX make

---

## 2. Building the Project

From the project root directory, run:

```bash
# 1. Configure the build directory (exports compilation database)
cmake -S . -B build

# 2. Compile the compiler and test binaries
cmake --build build
```

This creates:
- `./build/compiler`: The main compiler CLI executable
- `./build/compiler_tests`: The automated test suite executable

---

## 3. Running Automated Tests

Run the full suite of 25 unit and regression tests using CTest:

```bash
ctest --test-dir build --output-on-failure
```

To see individual test case names:
```bash
./build/compiler_tests
```

**Expected Result:**
```text
valid program: PASS
lexical error: PASS
syntax error: PASS
undeclared variable: PASS
duplicate declaration: PASS
IR and execution: PASS
runtime error: PASS
boolean/control flow: PASS
type/scope errors: PASS
optimization/target code: PASS
else branch: PASS
nested while loops: PASS
nested if blocks: PASS
empty block: PASS
missing closing brace: PASS
missing closing parenthesis: PASS
unterminated block comment: PASS
int plus bool: PASS
not on int: PASS
bool comparison: PASS
integer overflow: PASS
boolean equality: PASS
multiple diagnostics: PASS
optimization statistics: PASS
educational hints: PASS
```

---

## 4. Running Programs

### 4.1 Execute and Print Output
To interpret and execute an EduLang source file:
```bash
./build/compiler run examples/hello.edu
./build/compiler run examples/nested_loops.edu
```

### 4.2 Full Pipeline View (All Stages)
Run all stages with pass/fail tracking:
```bash
./build/compiler examples/hello.edu
```

---

## 5. Inspecting Individual Compiler Stages

EduLang allows developers and students to isolate and study each compiler phase:

| Phase | Command | Description |
|:---|:---|:---|
| **Lexical Tokens** | `./build/compiler tokens <file.edu>` | Scans input into tokens with line:col spans |
| **AST Hierarchy** | `./build/compiler ast <file.edu>` | Generates and prints the indented Abstract Syntax Tree |
| **Symbol Table** | `./build/compiler symbols <file.edu>` | Displays scoped identifiers, types, and declaration positions |
| **Intermediate Code** | `./build/compiler ir <file.edu>` | Displays lowered Three-Address Code (TAC) instructions |
| **Target Code** | `./build/compiler target <file.edu>` | Generates readable pseudo-assembly (`MOV`, `ADD`, `PRINT`) |
| **Execution** | `./build/compiler run <file.edu>` | Runs the program through the TAC runtime interpreter |
| **Performance Benchmark** | `./build/compiler bench <file.edu>` | Measures execution time across each phase and optimization rate |

---

## 6. Benchmarking & Performance Evaluation

To profile compiler phase latencies and constant folding optimization metrics:

```bash
./build/compiler bench examples/arithmetic_control.edu
```

**Sample Output:**
```text
========================================================
        EduLang Compiler Benchmark & Performance        
========================================================
Source File: examples/arithmetic_control.edu (65 bytes)

Pipeline Stage                       Time (us)     % Total
----------------------------------------------------------
1. Lexical Analysis                      22.46      20.43%
2. Syntax Parsing & AST                  10.25       9.33%
3. Semantic Analysis & Scopes            13.46      12.24%
4. TAC IR Generation                     23.79      21.65%
5. Optimization Pass                     19.38      17.63%
6. Target Code Generation                 5.50       5.00%
7. Runtime Execution                     11.71      10.65%
==========================================================
Total Pipeline Execution                109.92     100.00%

Optimization Metrics:
  - Tokens Generated:             31
  - Unoptimized TAC Instructions: 17
  - Optimized TAC Instructions:   17
  - Constant Expressions Folded:  1
  - Optimization Reduction Rate:  5.9%
```

---

## 7. Demonstrating Educational Error Detection

Run the test errors to demonstrate how EduLang pinpoints **WHERE**, **WHAT**, and **HOW TO FIX** errors:

```bash
# 1. Lexical Error (Invalid character)
./build/compiler examples/errors/lexical_error.edu

# 2. Syntax Error (Missing expression)
./build/compiler examples/errors/syntax_error.edu

# 3. Semantic Error (Undeclared variable)
./build/compiler examples/errors/semantic_error.edu

# 4. Semantic Error (Type mismatch)
./build/compiler examples/errors/type_mismatch.edu

# 5. Semantic Error (Duplicate declaration)
./build/compiler examples/errors/duplicate_declaration.edu

# 6. Semantic Error (Condition must be boolean)
./build/compiler examples/errors/invalid_condition.edu

# 7. Runtime Error (Division by zero)
./build/compiler examples/errors/runtime_error.edu
```
