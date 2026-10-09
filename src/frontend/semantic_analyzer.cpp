#include "edulang/frontend/semantic_analyzer.hpp"

#include <string>

#include "edulang/frontend/symbol_table.hpp"

namespace edulang {
namespace {

const char* type_name(ValueType type) {
    if (type == ValueType::Int) return "int";
    if (type == ValueType::Bool) return "bool";
    return "invalid";
}

// Arithmetic and comparisons use int operands; logical operators and ! use bool.
// Every expression returns its type so assignments and control-flow conditions can be checked.
ValueType analyze_expression(const ast::Expr& expression, const SymbolTable& symbols,
                            DiagnosticBag& diagnostics) {
    if (dynamic_cast<const ast::NumberExpr*>(&expression) != nullptr) return ValueType::Int;
    if (const auto* boolean = dynamic_cast<const ast::BooleanExpr*>(&expression)) {
        static_cast<void>(boolean);
        return ValueType::Bool;
    }
    if (const auto* name = dynamic_cast<const ast::NameExpr*>(&expression)) {
        const Symbol* symbol = symbols.lookup(name->name);
        if (symbol == nullptr) {
            diagnostics.error(name->span, "undeclared variable '" + name->name + "'");
            return ValueType::Error;
        }
        return symbol->type;
    }
    if (const auto* unary = dynamic_cast<const ast::UnaryExpr*>(&expression)) {
        const ValueType operand = analyze_expression(*unary->operand, symbols, diagnostics);
        const ValueType expected = unary->op == "-" ? ValueType::Int : ValueType::Bool;
        if (operand != ValueType::Error && operand != expected) {
            diagnostics.error(unary->span, "operator '" + unary->op + "' requires " + type_name(expected));
            return ValueType::Error;
        }
        return expected;
    }
    const auto* binary = dynamic_cast<const ast::BinaryExpr*>(&expression);
    if (binary == nullptr) return ValueType::Error;

    const ValueType left = analyze_expression(*binary->left, symbols, diagnostics);
    const ValueType right = analyze_expression(*binary->right, symbols, diagnostics);
    if (left == ValueType::Error || right == ValueType::Error) return ValueType::Error;

    if (binary->op == "+" || binary->op == "-" || binary->op == "*" || binary->op == "/") {
        if (left != ValueType::Int || right != ValueType::Int) {
            diagnostics.error(binary->span, "arithmetic operator requires int operands");
            return ValueType::Error;
        }
        return ValueType::Int;
    }
    if (binary->op == "<" || binary->op == "<=" || binary->op == ">" || binary->op == ">=") {
        if (left != ValueType::Int || right != ValueType::Int) {
            diagnostics.error(binary->span, "comparison operator requires int operands");
            return ValueType::Error;
        }
        return ValueType::Bool;
    }
    if (binary->op == "==" || binary->op == "!=") {
        if (left != right) {
            diagnostics.error(binary->span, "equality operands must have the same type");
            return ValueType::Error;
        }
        return ValueType::Bool;
    }
    if (binary->op == "&&" || binary->op == "||") {
        if (left != ValueType::Bool || right != ValueType::Bool) {
            diagnostics.error(binary->span, "logical operator requires bool operands");
            return ValueType::Error;
        }
        return ValueType::Bool;
    }
    return ValueType::Error;
}

void analyze_statement(const ast::Stmt& statement, SymbolTable& symbols,
                       DiagnosticBag& diagnostics) {
    if (const auto* declaration = dynamic_cast<const ast::DeclarationStmt*>(&statement)) {
        if (symbols.is_declared_in_current_scope(declaration->name)) {
            diagnostics.error(declaration->name_span, "variable '" + declaration->name + "' is already declared");
        } else {
            const ValueType value_type = analyze_expression(*declaration->value, symbols, diagnostics);
            if (value_type != ValueType::Error && value_type != ValueType::Int) {
                diagnostics.error(declaration->value->span, "declaration of int requires an int expression");
            }
            static_cast<void>(symbols.declare(declaration->name, Symbol{ValueType::Int, declaration->name_span}));
            return;
        }
        analyze_expression(*declaration->value, symbols, diagnostics);
        return;
    }
    if (const auto* assignment = dynamic_cast<const ast::AssignmentStmt*>(&statement)) {
        const Symbol* symbol = symbols.lookup(assignment->name);
        if (symbol == nullptr) {
            diagnostics.error(assignment->name_span, "undeclared variable '" + assignment->name + "'");
        }
        const ValueType value_type = analyze_expression(*assignment->value, symbols, diagnostics);
        if (symbol != nullptr && value_type != ValueType::Error && value_type != symbol->type) {
            diagnostics.error(assignment->value->span, "cannot assign " + std::string(type_name(value_type)) +
                                                        " to " + type_name(symbol->type));
        }
        return;
    }
    if (const auto* print = dynamic_cast<const ast::PrintStmt*>(&statement)) {
        analyze_expression(*print->value, symbols, diagnostics);
        return;
    }
    if (const auto* block = dynamic_cast<const ast::BlockStmt*>(&statement)) {
        symbols.enter_scope();
        for (const auto& nested : block->statements) analyze_statement(*nested, symbols, diagnostics);
        symbols.exit_scope();
        return;
    }
    if (const auto* conditional = dynamic_cast<const ast::IfStmt*>(&statement)) {
        if (analyze_expression(*conditional->condition, symbols, diagnostics) != ValueType::Bool) {
            diagnostics.error(conditional->condition->span, "if condition must be bool");
        }
        analyze_statement(*conditional->then_branch, symbols, diagnostics);
        if (conditional->else_branch) analyze_statement(*conditional->else_branch, symbols, diagnostics);
        return;
    }
    if (const auto* loop = dynamic_cast<const ast::WhileStmt*>(&statement)) {
        if (analyze_expression(*loop->condition, symbols, diagnostics) != ValueType::Bool) {
            diagnostics.error(loop->condition->span, "while condition must be bool");
        }
        analyze_statement(*loop->body, symbols, diagnostics);
    }
}

}  // namespace

void SemanticAnalyzer::analyze(const ast::Program& program, DiagnosticBag& diagnostics) const {
    SymbolTable symbols;
    for (const auto& statement : program.statements) analyze_statement(*statement, symbols, diagnostics);
}

}  // namespace edulang
