#include <iostream>
#include <sstream>

#include "edulang/frontend/frontend.hpp"
#include "edulang/ir/generator.hpp"
#include "edulang/ir/optimizer.hpp"
#include "edulang/runtime/interpreter.hpp"
#include "edulang/target/code_generator.hpp"

namespace {

bool test_valid_program() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    const auto result = frontend.analyze(
        "int a = 10;\nint b = 20;\na = a + b;\nprint(a);\n", diagnostics);
    return !diagnostics.has_errors() && result.program.statements.size() == 4 &&
           result.tokens[0].type == edulang::TokenType::KwInt;
}

bool test_lexical_error() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    static_cast<void>(frontend.analyze("int a = 10; @\n", diagnostics));
    return diagnostics.has_errors() && diagnostics.all()[0].span.start.line == 1;
}

bool test_syntax_error() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    static_cast<void>(frontend.analyze("int a = ;\n", diagnostics));
    return diagnostics.has_errors() && diagnostics.all()[0].span.start.line == 1;
}

bool test_undeclared_variable() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    static_cast<void>(frontend.analyze("print(a);\n", diagnostics));
    return diagnostics.has_errors() && diagnostics.all()[0].message.find("undeclared") != std::string::npos;
}

bool test_duplicate_declaration() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    static_cast<void>(frontend.analyze("int a = 1;\nint a = 2;\n", diagnostics));
    return diagnostics.has_errors() && diagnostics.all()[0].message.find("already declared") != std::string::npos;
}

bool test_intermediate_code_and_execution() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    const auto result = frontend.analyze("int a = 10 + 2;\nprint(a * 3);\n", diagnostics);
    if (diagnostics.has_errors()) {
        return false;
    }

    edulang::ir::Generator generator;
    const edulang::ir::Program ir = generator.generate(result.program);
    std::ostringstream output;
    edulang::runtime::Interpreter interpreter;
    return interpreter.run(ir, diagnostics, output) && output.str() == "36\n";
}

bool test_runtime_error() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    const auto result = frontend.analyze("print(10 / 0);\n", diagnostics);
    if (diagnostics.has_errors()) {
        return false;
    }

    edulang::ir::Generator generator;
    const edulang::ir::Program ir = generator.generate(result.program);
    std::ostringstream output;
    edulang::runtime::Interpreter interpreter;
    return !interpreter.run(ir, diagnostics, output) && diagnostics.has_errors() &&
           diagnostics.all()[0].message == "division by zero";
}

bool test_boolean_control_flow_and_scopes() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    const auto result = frontend.analyze(
        "int a = 0;\nwhile (a < 3) { print(a); a = a + 1; }\n"
        "if (a == 3) { print(true); } else { print(false); }\n", diagnostics);
    if (diagnostics.has_errors()) return false;

    edulang::ir::Generator generator;
    edulang::ir::Optimizer optimizer;
    const edulang::ir::Program ir = optimizer.optimize(generator.generate(result.program));
    std::ostringstream output;
    edulang::runtime::Interpreter interpreter;
    return interpreter.run(ir, diagnostics, output) && output.str() == "0\n1\n2\ntrue\n";
}

bool test_type_and_scope_errors() {
    edulang::DiagnosticBag type_diagnostics;
    edulang::Frontend frontend;
    static_cast<void>(frontend.analyze("int a = true;\n", type_diagnostics));
    if (!type_diagnostics.has_errors() || type_diagnostics.all()[0].message.find("requires an int") == std::string::npos) {
        return false;
    }

    edulang::DiagnosticBag scope_diagnostics;
    static_cast<void>(frontend.analyze("if (true) { int inner = 1; print(inner); } print(inner);\n", scope_diagnostics));
    return scope_diagnostics.has_errors() && scope_diagnostics.all().back().message.find("undeclared") != std::string::npos;
}

bool test_optimization_and_target_code() {
    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    const auto result = frontend.analyze("print(2 + 3 * 4);\n", diagnostics);
    if (diagnostics.has_errors()) return false;

    edulang::ir::Generator generator;
    edulang::ir::Optimizer optimizer;
    const edulang::ir::Program ir = optimizer.optimize(generator.generate(result.program));
    std::ostringstream target;
    edulang::target::CodeGenerator code_generator;
    code_generator.generate(ir, target);
    return target.str().find("MOV t") != std::string::npos && target.str().find("PRINT") != std::string::npos;
}
}
int main() {
    const bool valid = test_valid_program();
    const bool lexical = test_lexical_error();
    const bool syntax = test_syntax_error();
    const bool undeclared = test_undeclared_variable();
    const bool duplicate = test_duplicate_declaration();
    const bool execution = test_intermediate_code_and_execution();
    const bool runtime = test_runtime_error();
    const bool control_flow = test_boolean_control_flow_and_scopes();
    const bool type_scope = test_type_and_scope_errors();
    const bool optimization = test_optimization_and_target_code();

    std::cout << "valid program: " << (valid ? "PASS" : "FAIL") << '\n';
    std::cout << "lexical error: " << (lexical ? "PASS" : "FAIL") << '\n';
    std::cout << "syntax error: " << (syntax ? "PASS" : "FAIL") << '\n';
    std::cout << "undeclared variable: " << (undeclared ? "PASS" : "FAIL") << '\n';
    std::cout << "duplicate declaration: " << (duplicate ? "PASS" : "FAIL") << '\n';
    std::cout << "IR and execution: " << (execution ? "PASS" : "FAIL") << '\n';
    std::cout << "runtime error: " << (runtime ? "PASS" : "FAIL") << '\n';
    std::cout << "boolean/control flow: " << (control_flow ? "PASS" : "FAIL") << '\n';
    std::cout << "type/scope errors: " << (type_scope ? "PASS" : "FAIL") << '\n';
    std::cout << "optimization/target code: " << (optimization ? "PASS" : "FAIL") << '\n';
    return valid && lexical && syntax && undeclared && duplicate && execution && runtime &&
           control_flow && type_scope && optimization ? 0 : 1;
}
