#include <iostream>

#include "edulang/frontend/frontend.hpp"

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
}
int main() {
    const bool valid = test_valid_program();
    const bool lexical = test_lexical_error();
    const bool syntax = test_syntax_error();

    std::cout << "valid program: " << (valid ? "PASS" : "FAIL") << '\n';
    std::cout << "lexical error: " << (lexical ? "PASS" : "FAIL") << '\n';
    std::cout << "syntax error: " << (syntax ? "PASS" : "FAIL") << '\n';
    return valid && lexical && syntax ? 0 : 1;
}
