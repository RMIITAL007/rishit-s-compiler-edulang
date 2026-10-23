# EduLang Compiler: Final Review Presentation & Viva Guide
**Course:** BCSE307P - Compiler Design Laboratory  
**Evaluation:** Review 3 (35 Marks) + Overall Assessment (100 Marks Total)  

---

## 1. Review 3 Presentation Structure (Slide-by-Slide Outline)

### Slide 1: Title & Project Overview
- **Title:** EduLang: An Educational C++20 Compiler
- **Student Name:** Rishit Mittal
- **Core Vision:** Bringing transparency to compiler internals with stage-by-stage inspection, static type safety, constant-folding optimization, and human-centered educational diagnostics with fix hints.

### Slide 2: Problem Statement & Motivation
- **The Problem:** Novice programmers face cryptic, one-line compiler errors without contextual help or suggestions.
- **The Solution:** A fully transparent 9-stage compiler pipeline that answers:
  1. *Which analysis phase caught the issue?*
  2. *Where is it?* (Line, column, source line snippet with carets `^^^^`)
  3. *What is the mistake?* (Plain-English explanation)
  4. *How to fix it?* (`= hint: ...`)

### Slide 3: Language Features & Grammar
- Variable declarations (`int x = 10;`) and assignments (`x = x + 1;`)
- Booleans (`true`, `false`) and logical operators (`&&`, `||`, `!`)
- Relational operators (`<`, `<=`, `>`, `>=`, `==`, `!=`)
- Control flow: `if`, `else`, and `while` loops with nested `{}` blocks
- I/O: `print(expression);`

### Slide 4: System Architecture & Data Flow
- Show the 9-stage pipeline diagram:
  `Source Code -> Lexer -> Tokens -> Parser -> AST -> Semantic Analyzer & Scoped Symbols -> TAC IR Generator -> Optimizer (Constant Folding) -> Target Pseudo-Assembly & Interpreter VM`

### Slide 5: Frontend Implementation (Lexer & Parser)
- **Lexer:** Deterministic scanner with single-pass character consumption and precise `SourceSpan` tracking.
- **Parser:** Recursive descent with **Operator Precedence Climbing** (8 tiers of precedence from logical OR down to unary/primary expressions).

### Slide 6: Semantic Analysis & Scoped Symbol Table
- **Scope Stack:** `std::vector<std::unordered_map<std::string, Symbol>>` representing block scope hierarchies.
- **Rules Enforced:**
  - Strict type checking (`int` vs `bool`)
  - No duplicate declarations in the same scope
  - Proper condition typing (`if` and `while` require `bool`)
  - Out-of-scope and undeclared variable detection

### Slide 7: Intermediate Representation & Optimization
- **Three-Address Code (TAC):** Virtual temporaries (`t0`, `t1`, ...), labels, conditional branches (`jump_if_false`), and memory loads/stores.
- **Optimizer:** Constant folding over TAC instructions, tracking instructions folded and the percentage of TAC instructions optimized via constant folding.

### Slide 8: Intermediate Representation & Optimization
- **Three-Address Code (TAC):** Virtual temporaries, labels, conditional branches, and resolved storage names.
- **Optimizer:** Checked constant folding over TAC instructions with tracked metrics.

### Slide 9: Target Code Generation & Runtime Engine
- **Target Pseudo-Assembly:** `MOV`, `ADD`, `SUB`, `JMP`, `PRINT`.
- **Virtual Machine:** Executes optimized TAC instructions and reports runtime diagnostics such as division by zero.

### Slide 10: Diagnostics & Test Verification
- **31 Automated Unit Tests & 6 Registered CTest Targets (100% Pass Rate)**
- **Educational Diagnostics:** Source spans, carets, and actionable hints across lexical, syntax, semantic, and runtime failures.

### Slide 11: Performance & Known Limitations
- **High-Resolution Benchmark:** Phase breakdown and constant-folding metrics via `compiler bench`.
- **Scope:** Pseudo-assembly target, `int`/`bool` types, and constant folding only.

### Slide 12: Conclusion & Future Scope
- User-defined functions, float/string data types, and LLVM backend integration.

---

## 2. Live Demo Script (Step-by-Step for Evaluation)

```bash
# 1. Show Clean Build & Test Suite (Testing: 4 Marks)
cmake --build build
ctest --test-dir build --output-on-failure
./build/compiler_tests

# 2. Show Full End-to-End Pipeline (Completion: 6 Marks)
./build/compiler examples/hello.edu

# 3. Show Complex Nested Control Flow (Complexity: 5 Marks)
./build/compiler run examples/nested_loops.edu

# 4. Show Performance Profiling & Optimization Metrics (Performance: Section 12.2)
./build/compiler bench examples/arithmetic_control.edu

# 5. Show Educational Error Diagnostics & Hints (Innovation: 4 Marks)
./build/compiler examples/errors/semantic_error.edu
./build/compiler examples/errors/type_mismatch.edu
./build/compiler examples/errors/invalid_condition.edu
./build/compiler examples/errors/runtime_error.edu
```

---

## 3. Review 3 Viva Questions & Model Answers (4 Marks)

### Q1: Why did you choose Recursive Descent with Operator Precedence?
**Answer:**  
Recursive descent provides direct, readable control over error handling and synchronizing tokens upon encountering a syntax error. By pairing it with operator precedence climbing, we avoid deep recursive call overhead for binary operators, correctly grouping arithmetic (`*`, `/` over `+`, `-`), comparisons (`<`, `>`), equality (`==`, `!=`), and logical operators (`&&`, `||`).

### Q2: How does your Symbol Table support lexical scoping and shadowing?
**Answer:**  
The symbol table uses a stack of hash maps (`std::vector<std::unordered_map<std::string, Symbol>>`). When the parser enters a `{` block, `enter_scope()` pushes a new map onto the stack. When resolving a variable name, `lookup()` searches from the top of the stack (innermost scope) down to the bottom (global scope). When the block exits `}`, `exit_scope()` pops the innermost scope. This naturally supports variable shadowing while strictly forbidding redeclaration within the *same* scope.

### Q3: What intermediate code representation did you select and why?
**Answer:**  
We chose **Three-Address Code (TAC)**. TAC linearizes the tree structure of the AST into a sequence of atomic operations with at most one operator and two operands, utilizing virtual temporaries (`t0`, `t1`). TAC is the standard textbook intermediate form because it simplifies machine-independent optimizations (like constant folding and dead code elimination) and maps directly to register-based target assembly.

### Q4: How does your constant folding optimization pass operate?
**Answer:**  
The optimizer inspects the sequence of TAC instructions. It maintains an active map of temporaries bound to constant integers or booleans. When it encounters an operation where both operands are known constants (e.g., `ADD t2, t0, t1` where `t0=10` and `t1=20`), it computes the result at compile time and emits `LOAD_CONST t2, 30`. When encountering control-flow jumps or labels, the constant map is invalidated to prevent unsafe cross-block propagation. It also records statistics on instructions folded and the percentage of TAC instructions optimized via constant folding (`constants_folded / instructions_before`, evaluating to 5.9% in the benchmark where 1 of 17 instructions is folded into a constant load).

### Q5: How do your educational diagnostics differ from typical compilers?
**Answer:**  
Standard compilers often emit cryptic errors or point past the offending token. EduLang retains exact `SourceSpan` coordinates (line and column) across every AST node and TAC instruction. When an error is encountered, it prints the original line, draws exact caret pointers (`^^^^`) under the token, and attaches an actionable `= hint:` suggesting how to resolve the mistake.
