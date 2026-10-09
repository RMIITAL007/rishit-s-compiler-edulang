#include "edulang/ast/printer.hpp"

namespace edulang::ast {
namespace {

void indent(std::ostream& output, int level) {
    for (int i = 0; i < level; ++i) {
        output << "  ";
    }
}

void print_expression(const Expr& expression, std::ostream& output, int level) {
    if (const auto* number = dynamic_cast<const NumberExpr*>(&expression)) {
        indent(output, level);
        output << "Number(" << number->value << ")\n";
    } else if (const auto* boolean = dynamic_cast<const BooleanExpr*>(&expression)) {
        indent(output, level);
        output << "Boolean(" << (boolean->value ? "true" : "false") << ")\n";
    } else if (const auto* name = dynamic_cast<const NameExpr*>(&expression)) {
        indent(output, level);
        output << "Name(" << name->name << ")\n";
    } else if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expression)) {
        indent(output, level);
        output << "Binary(" << binary->op << ")\n";
        print_expression(*binary->left, output, level + 1);
        print_expression(*binary->right, output, level + 1);
    } else if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expression)) {
        indent(output, level);
        output << "Unary(" << unary->op << ")\n";
        print_expression(*unary->operand, output, level + 1);
    }
}

void print_statement(const Stmt& statement, std::ostream& output, int level) {
    if (const auto* declaration = dynamic_cast<const DeclarationStmt*>(&statement)) {
        indent(output, level);
        output << "Declaration(" << declaration->name << ")\n";
        print_expression(*declaration->value, output, level + 1);
    } else if (const auto* assignment = dynamic_cast<const AssignmentStmt*>(&statement)) {
        indent(output, level);
        output << "Assignment(" << assignment->name << ")\n";
        print_expression(*assignment->value, output, level + 1);
    } else if (const auto* print = dynamic_cast<const PrintStmt*>(&statement)) {
        indent(output, level);
        output << "Print\n";
        print_expression(*print->value, output, level + 1);
    } else if (const auto* block = dynamic_cast<const BlockStmt*>(&statement)) {
        indent(output, level);
        output << "Block\n";
        for (const StmtPtr& nested : block->statements) print_statement(*nested, output, level + 1);
    } else if (const auto* conditional = dynamic_cast<const IfStmt*>(&statement)) {
        indent(output, level);
        output << "If\n";
        print_expression(*conditional->condition, output, level + 1);
        print_statement(*conditional->then_branch, output, level + 1);
        if (conditional->else_branch) print_statement(*conditional->else_branch, output, level + 1);
    } else if (const auto* loop = dynamic_cast<const WhileStmt*>(&statement)) {
        indent(output, level);
        output << "While\n";
        print_expression(*loop->condition, output, level + 1);
        print_statement(*loop->body, output, level + 1);
    }
}

}  // namespace

void print_ast(const Program& program, std::ostream& output) {
    output << "Program\n";
    for (const StmtPtr& statement : program.statements) {
        print_statement(*statement, output, 1);
    }
}

}  // namespace edulang::ast
