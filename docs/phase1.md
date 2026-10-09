# EduLang Compiler: Phase 1

## 1. Project Title

**EduLang Compiler: Lexical and Syntax Analysis with AST Generation**

## 2. Abstract

This project is a Phase 1 compiler frontend for a small language called EduLang. The current implementation reads a source file, breaks it into tokens, checks the syntax of supported statements and expressions, builds an abstract syntax tree (AST), and prints tokens, the AST, or diagnostics through a command-line program. Phase 1 is focused on lexical analysis, syntax analysis, AST generation, and basic error reporting.

## 3. Problem Statement

A compiler needs to understand source code before later stages can process it. The aim of this phase is to build the initial frontend for EduLang so that source text can be converted into meaningful tokens and a structured AST. The compiler should also identify invalid characters and invalid syntax with useful source locations.

## 4. Motivation

Compiler Design concepts are easier to understand when they are implemented in a small language. EduLang keeps the first phase manageable while still showing the main frontend flow: source code is scanned, parsed according to grammar rules, represented as a tree, and reported when errors are found.

## 5. Objectives

- Define a small language with integer declarations, assignments, print statements, and arithmetic expressions.
- Implement a lexer that recognizes the tokens used by the language.
- Implement a parser for the supported statement and expression syntax.
- Generate an AST for valid input.
- Report lexical and syntax errors with line and column information.
- Provide a command-line interface for viewing tokens and AST output.
- Add basic automated tests for valid input, lexical errors, and syntax errors.

## 6. Scope of Phase 1

Phase 1 covers the compiler frontend up to AST generation. It includes source reading, lexical analysis, token generation, syntax analysis, AST construction, AST printing, and diagnostic reporting. Review 2 extends this frontend with semantic analysis, a symbol table, intermediate code, and an execution engine; the Review 2 implementation is documented in `docs/review2.md`.

## 7. Background / Compiler Design Concepts

The lexer performs lexical analysis by reading characters and grouping them into tokens such as keywords, identifiers, numbers, operators, and delimiters. The parser performs syntax analysis by consuming the token sequence according to the language grammar. The parser builds an AST so that the structure of declarations, assignments, print statements, and expressions is represented independently of the original formatting. Diagnostics store errors with source spans and render the source line with a caret pointing to the problem.

## 8. Proposed Programming Language

### Language name

The language is called **EduLang**.

### Purpose

EduLang is a small teaching language for demonstrating the first stages of a compiler. In Phase 1, it supports integer variables, assignments, printing expressions, and basic arithmetic.

### Basic syntax

Statements end with a semicolon. Variables can be declared with `int`, assigned with `=`, and used in expressions. A print statement has the form `print(expression);`.

```text
int total = 10 + 20;
total = total * 2;
print(total);
```

The lexer also skips whitespace, line comments beginning with `//`, and block comments enclosed by `/*` and `*/`.

### Keywords

The implemented keywords are `int` and `print`.

### Identifiers

An identifier starts with an uppercase letter, lowercase letter, or underscore. The remaining characters may also include digits. Examples are `a`, `total`, and `_value`.

### Literals

The implemented literal is a decimal integer literal made of one or more digits. Integer values are stored as signed 64-bit values. Out-of-range integer literals are reported as lexical errors.

### Operators

The implemented operators are:

- Arithmetic: `+`, `-`, `*`, `/`
- Assignment: `=`

The parser gives `*` and `/` higher precedence than `+` and `-`. Parentheses can be used to group expressions. Review 2 extends the expression grammar with unary operators, boolean operators, comparisons, and boolean literals.

### Delimiters

The implemented delimiters are `(`, `)`, and `;`.

## 9. Language Specification

### Tokens

The lexer produces the following token types:

| Category | Tokens |
| --- | --- |
| Special | End of file |
| Names and values | Identifier, Number |
| Keywords | `int`, `print` |
| Delimiters | `(`, `)`, `;` |
| Operators | `+`, `-`, `*`, `/`, `=` |

Comments and whitespace are recognized while scanning but are not emitted as tokens.

### Grammar / syntax rules actually supported

The parser implements the following grammar:

```text
program         -> statement* EOF
statement       -> declaration | assignment | print_statement
declaration     -> "int" identifier "=" expression ";"
assignment      -> identifier "=" expression ";"
print_statement -> "print" "(" expression ")" ";"

expression      -> term (("+" | "-") term)*
term            -> factor (("*" | "/") factor)*
factor          -> number | identifier | "(" expression ")"
```

The grammar describes syntax only. The current implementation does not check whether a variable was declared before use or whether a declaration is duplicated.

## 10. Phase 1 Methodology

1. **Source code:** The CLI reads the complete source file into a string.
2. **Lexical analysis:** `Lexer` scans the characters, skips whitespace and comments, recognizes keywords and other token types, and records source spans.
3. **Token generation:** The lexer returns a vector of tokens and always appends an end-of-file token. Invalid characters and invalid integer ranges are added to the diagnostic bag.
4. **Syntax analysis:** `Parser` consumes the token vector and recognizes declarations, assignments, print statements, and arithmetic expressions. It uses separate parsing functions for expressions, terms, and factors to implement operator precedence.
5. **AST generation:** The parser creates a `Program` containing `DeclarationStmt`, `AssignmentStmt`, and `PrintStmt` nodes. Expressions are represented by `NumberExpr`, `NameExpr`, and `BinaryExpr` nodes.
6. **Error reporting:** Lexical and syntax errors are stored in `DiagnosticBag`. The CLI renders the source filename, line, column, message, source line, and a caret. When diagnostics contain errors, the CLI returns exit code 1 and does not print the AST.

## 11. System Architecture

The Phase 1 pipeline is:

```text
Source Program
    -> Lexer
    -> Tokens
    -> Parser
    -> AST
    -> AST Printer / Diagnostics
```

`Frontend::analyze` connects the lexer and parser. The command-line executable then displays the token list and, for error-free input, the AST. Diagnostics can be produced by either the lexer or parser.

## 12. Module Description

- **Token module:** Defines token types, token lexemes, source spans, and optional integer values. It also provides readable token names.
- **Lexer:** Scans source characters, recognizes identifiers, keywords, numbers, operators, delimiters, whitespace, and comments, and reports invalid characters.
- **Parser:** Parses the token stream using functions for statements, expressions, terms, and factors. It builds AST nodes and performs basic error synchronization at semicolons.
- **AST:** Defines the program, statement nodes, and expression nodes used to represent the parsed structure.
- **AST printer:** Prints the AST in an indented tree format such as `Program`, `Declaration`, `Print`, `Binary`, `Number`, and `Name`.
- **Diagnostic/error handling:** Stores errors and warnings with source spans and renders them with file, line, column, message, and a source caret. Phase 1 currently generates errors; the warning API exists but is not used by the frontend.
- **CLI:** `compiler` reads one source file. With no mode it prints tokens and, when there are no errors, the AST. It also supports `tokens` and `ast` modes and returns non-zero status for invalid command usage, file errors, or diagnostics.

## 13. Technology Stack

- **C++:** C++20 implementation language.
- **CMake:** CMake 3.20 or newer is used to configure the project and build the `compiler_frontend`, `compiler`, and `compiler_tests` targets.
- **Testing framework/tools:** CTest runs the configured `compiler_tests` executable. The test executable contains C++ checks for a valid program, a lexical error, and a syntax error. The command-line demos are also used to verify the three Phase 1 cases.

## 14. Project Structure

```text
include/edulang/
  ast/          AST and AST printer interfaces
  common/       Source position and span types
  frontend/     Tokens, lexer, parser, diagnostics, and frontend interface
src/
  ast/          AST printer implementation
  frontend/     Lexer, parser, diagnostics, and frontend implementations
  main.cpp      Command-line compiler
tests/
  test_main.cpp Basic frontend tests
examples/
  hello.edu               Valid example
  nested_boolean.edu      Lexical-error example
  arithmetic_control.edu  Syntax-error example
docs/
  phase1.md               This Phase 1 documentation
CMakeLists.txt             Build and test configuration
```

## 15. Phase 1 Implementation

The current implementation supports integer declarations such as `int a = 10;`, assignments such as `a = a + 1;`, and print statements such as `print(a);`. It supports integer and identifier expressions, parentheses, and the four binary arithmetic operators with normal arithmetic precedence. The lexer handles line and block comments, tracks line and column positions, and appends an EOF token.

The parser creates an AST and the CLI can print its tree form. For valid input, the compiler prints the token list followed by the AST, intermediate code, and execution output. For invalid input, it prints the tokens collected before or around the error and renders diagnostics. Basic semantic analysis and execution are implemented for Review 2.

## 16. Testing

The following commands were run against the current repository:

```text
cmake --build build
```

Result: **PASS**. All configured targets built successfully.

```text
ctest --test-dir build --output-on-failure
```

Result: **PASS**. CTest reported `100% tests passed out of 1`.

### Valid program

Command:

```text
build/compiler examples/hello.edu
```

The file contains two integer declarations and `print(a + b);`. Result: **PASS**. The compiler printed tokens, followed by an AST containing `Declaration(a)`, `Declaration(b)`, and `Print` with `Binary(+)` and the names `a` and `b`.

### Lexical error case

Command:

```text
build/compiler examples/nested_boolean.edu
```

The file currently contains the declaration `int a = 10;` followed by the invalid character `@`. The filename is `nested_boolean.edu`, but this example does not contain a nested Boolean expression. Result: **PASS**. The compiler reported `unexpected character '@'` at line 2, column 1 and returned an error status.

### Additional valid example

Command:

```text
build/compiler examples/arithmetic_control.edu
```

The file contains `int a = 5;`. Result: **PASS**. The compiler printed the tokens and an AST containing `Declaration(a)` with `Number(5)`, and returned a successful status.

There is currently no syntax-error example file in the `examples/` directory. The syntax-error behavior is covered by the automated test in `tests/test_main.cpp`, which checks the invalid input `int a = ;` and confirms that a diagnostic is generated.

## 17. Phase 1 Results

Phase 1 successfully builds a working frontend for the current EduLang syntax. It tokenizes source programs, parses valid declarations, assignments, print statements, and arithmetic expressions, produces an AST, and reports the tested lexical and syntax errors with source locations. The automated test and all three current example commands passed, with `nested_boolean.edu` producing the expected lexical diagnostic.

## 18. Current Limitations

- Semantic analysis currently supports undeclared-variable and duplicate-declaration checks.
- The compiler executes supported programs through an intermediate-code interpreter, but does not generate machine code.
- The symbol table currently tracks integer symbols and declaration spans; nested block syntax is not yet part of the language.
- The current language supports integer and boolean expressions, `if`/`else`, `while`, and nested brace blocks; functions and strings are not implemented.
- Unary minus is not parsed even though the `-` binary operator is supported.
- Diagnostics are basic and there is no recovery beyond parser synchronization at semicolons.

## 19. Work Planned for Phase 2

Phase 2 is implemented in `docs/review2.md` and includes semantic analysis, symbol-table management, declaration and use checking, three-address-style intermediate code, execution, runtime diagnostics, and automated tests. Future work includes richer types, control flow, optimization, and target-machine code generation.