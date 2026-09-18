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
    } else if (const auto* name = dynamic_cast<const NameExpr*>(&expression)) {
        indent(output, level);
        output << "Name(" << name->name << ")\n";
    } else if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expression)) {
        indent(output, level);
        output << "Binary(" << binary->op << ")\n";
        print_expression(*binary->left, output, level + 1);
        print_expression(*binary->right, output, level + 1);
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
