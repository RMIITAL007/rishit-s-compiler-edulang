#include "edulang/frontend/semantic_analyzer.hpp"

#include <string>
#include <unordered_map>

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
ValueType analyze_expression(ast::Expr& expression, const SymbolTable& symbols,
                            DiagnosticBag& diagnostics) {
    if (dynamic_cast<ast::NumberExpr*>(&expression) != nullptr) return ValueType::Int;
    if (const auto* boolean = dynamic_cast<ast::BooleanExpr*>(&expression)) {
        static_cast<void>(boolean);
        return ValueType::Bool;
    }
    if (auto* name = dynamic_cast<ast::NameExpr*>(&expression)) {
        const Symbol* symbol = symbols.lookup(name->name);
        if (symbol == nullptr) {
            diagnostics.error(name->span, "undeclared variable '" + name->name + "'",
                              "declare '" + name->name + "' first (e.g., 'int " + name->name + " = 0;').");
            return ValueType::Error;
        }
        name->storage_name = symbol->storage_name;
        return symbol->type;
    }
    if (auto* unary = dynamic_cast<ast::UnaryExpr*>(&expression)) {
        const ValueType operand = analyze_expression(*unary->operand, symbols, diagnostics);
        const ValueType expected = unary->op == "-" ? ValueType::Int : ValueType::Bool;
        if (operand != ValueType::Error && operand != expected) {
            diagnostics.error(unary->span, "operator '" + unary->op + "' requires " + type_name(expected),
                              "unary '" + unary->op + "' only operates on " + std::string(type_name(expected)) + " expressions.");
            return ValueType::Error;
        }
        return expected;
    }
    const auto* binary = dynamic_cast<ast::BinaryExpr*>(&expression);
    if (binary == nullptr) return ValueType::Error;

    const ValueType left = analyze_expression(*binary->left, symbols, diagnostics);
    const ValueType right = analyze_expression(*binary->right, symbols, diagnostics);
    if (left == ValueType::Error || right == ValueType::Error) return ValueType::Error;

    if (binary->op == "+" || binary->op == "-" || binary->op == "*" || binary->op == "/") {
        if (left != ValueType::Int || right != ValueType::Int) {
            diagnostics.error(binary->span, "arithmetic operator requires int operands",
                              "operators '+', '-', '*', and '/' require integer numbers on both sides.");
            return ValueType::Error;
        }
        return ValueType::Int;
    }
    if (binary->op == "<" || binary->op == "<=" || binary->op == ">" || binary->op == ">=") {
        if (left != ValueType::Int || right != ValueType::Int) {
            diagnostics.error(binary->span, "comparison operator requires int operands",
                              "relational comparison operators ('<', '<=', '>', '>=') compare numeric values.");
            return ValueType::Error;
        }
        return ValueType::Bool;
    }
    if (binary->op == "==" || binary->op == "!=") {
        if (left != right) {
            diagnostics.error(binary->span, "equality operands must have the same type",
                              "cannot compare different types; both sides must be int or both bool.");
            return ValueType::Error;
        }
        return ValueType::Bool;
    }
    if (binary->op == "&&" || binary->op == "||") {
        if (left != ValueType::Bool || right != ValueType::Bool) {
            diagnostics.error(binary->span, "logical operator requires bool operands",
                              "logical operators '&&' and '||' only operate on boolean expressions.");
            return ValueType::Error;
        }
        return ValueType::Bool;
    }
    return ValueType::Error;
}

std::string storage_name_for(const std::string& name,
                             std::unordered_map<std::string, std::size_t>& declaration_counts) {
    const std::size_t count = declaration_counts[name]++;
    return count == 0 ? name : name + "$" + std::to_string(count);
}

void analyze_statement(ast::Stmt& statement, SymbolTable& symbols,
                       DiagnosticBag& diagnostics,
                       std::unordered_map<std::string, std::size_t>& declaration_counts) {
    if (auto* declaration = dynamic_cast<ast::DeclarationStmt*>(&statement)) {
        if (symbols.is_declared_in_current_scope(declaration->name)) {
            diagnostics.error(declaration->name_span, "variable '" + declaration->name + "' is already declared",
                              "choose a unique name or assign to the existing variable without re-declaring 'int'.");
        } else {
            const ValueType value_type = analyze_expression(*declaration->value, symbols, diagnostics);
            if (value_type != ValueType::Error && value_type != ValueType::Int) {
                diagnostics.error(declaration->value->span, "declaration of int requires an int expression",
                                  "the variable is typed as 'int', but the assigned expression evaluates to 'bool'.");
            }
            declaration->storage_name = storage_name_for(declaration->name, declaration_counts);
            static_cast<void>(symbols.declare(
                declaration->name,
                Symbol{ValueType::Int, declaration->name_span, declaration->storage_name}));
            return;
        }
        analyze_expression(*declaration->value, symbols, diagnostics);
        return;
    }
    if (auto* assignment = dynamic_cast<ast::AssignmentStmt*>(&statement)) {
        const Symbol* symbol = symbols.lookup(assignment->name);
        if (symbol == nullptr) {
            diagnostics.error(assignment->name_span, "undeclared variable '" + assignment->name + "'",
                              "declare '" + assignment->name + "' before assigning to it (e.g., 'int " + assignment->name + " = 0;').");
        } else {
            assignment->storage_name = symbol->storage_name;
        }
        const ValueType value_type = analyze_expression(*assignment->value, symbols, diagnostics);
        if (symbol != nullptr && value_type != ValueType::Error && value_type != symbol->type) {
            diagnostics.error(assignment->value->span, "cannot assign " + std::string(type_name(value_type)) +
                                                        " to " + type_name(symbol->type),
                              "target variable '" + assignment->name + "' is of type " + std::string(type_name(symbol->type)) + "; assign an expression of matching type.");
        }
        return;
    }
    if (auto* print = dynamic_cast<ast::PrintStmt*>(&statement)) {
        analyze_expression(*print->value, symbols, diagnostics);
        return;
    }
    if (auto* block = dynamic_cast<ast::BlockStmt*>(&statement)) {
        symbols.enter_scope();
        for (const auto& nested : block->statements) {
            analyze_statement(*nested, symbols, diagnostics, declaration_counts);
        }
        symbols.exit_scope();
        return;
    }
    if (auto* conditional = dynamic_cast<ast::IfStmt*>(&statement)) {
        if (analyze_expression(*conditional->condition, symbols, diagnostics) != ValueType::Bool) {
            diagnostics.error(conditional->condition->span, "if condition must be bool",
                              "use a relational comparison (such as 'x > 0' or 'x == 1') or a boolean literal/variable.");
        }
        analyze_statement(*conditional->then_branch, symbols, diagnostics, declaration_counts);
        if (conditional->else_branch) {
            analyze_statement(*conditional->else_branch, symbols, diagnostics, declaration_counts);
        }
        return;
    }
    if (auto* loop = dynamic_cast<ast::WhileStmt*>(&statement)) {
        if (analyze_expression(*loop->condition, symbols, diagnostics) != ValueType::Bool) {
            diagnostics.error(loop->condition->span, "while condition must be bool",
                              "loops require a boolean condition expression (e.g., 'i < 10') to know when to repeat.");
        }
        analyze_statement(*loop->body, symbols, diagnostics, declaration_counts);
    }
}

}  // namespace

void SemanticAnalyzer::analyze(ast::Program& program, DiagnosticBag& diagnostics,
                               SymbolTable& symbols) const {
    std::unordered_map<std::string, std::size_t> declaration_counts;
    for (const auto& statement : program.statements) {
        analyze_statement(*statement, symbols, diagnostics, declaration_counts);
    }
}

}  // namespace edulang
