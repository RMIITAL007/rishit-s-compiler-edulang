#include "edulang/ir/generator.hpp"

#include <stdexcept>
#include <string>

namespace edulang::ir {

Program Generator::generate(const ast::Program& program) {
    program_.instructions.clear();
    temporary_index_ = 0;
    label_index_ = 0;
    for (const auto& statement : program.statements) generate_statement(*statement);
    return program_;
}

void Generator::generate_statement(const ast::Stmt& statement) {
    if (const auto* declaration = dynamic_cast<const ast::DeclarationStmt*>(&statement)) {
        const std::string value = generate_expression(*declaration->value);
        const std::string& storage_name = declaration->storage_name.empty()
            ? declaration->name
            : declaration->storage_name;
        program_.instructions.push_back({OpCode::StoreName, storage_name, value, {}, 0, false, {}, declaration->span});
    } else if (const auto* assignment = dynamic_cast<const ast::AssignmentStmt*>(&statement)) {
        const std::string value = generate_expression(*assignment->value);
        const std::string& storage_name = assignment->storage_name.empty()
            ? assignment->name
            : assignment->storage_name;
        program_.instructions.push_back({OpCode::StoreName, storage_name, value, {}, 0, false, {}, assignment->span});
    } else if (const auto* print = dynamic_cast<const ast::PrintStmt*>(&statement)) {
        const std::string value = generate_expression(*print->value);
        program_.instructions.push_back({OpCode::Print, {}, value, {}, 0, false, {}, print->span});
    } else if (const auto* block = dynamic_cast<const ast::BlockStmt*>(&statement)) {
        for (const auto& nested : block->statements) generate_statement(*nested);
    } else if (const auto* conditional = dynamic_cast<const ast::IfStmt*>(&statement)) {
        // if/else branches share an end label; a false condition jumps to else.
        const std::string condition = generate_expression(*conditional->condition);
        const std::string else_label = new_label();
        const std::string end_label = new_label();
        program_.instructions.push_back({OpCode::JumpIfFalse, {}, condition, {}, 0, false, else_label, conditional->condition->span});
        generate_statement(*conditional->then_branch);
        program_.instructions.push_back({OpCode::Jump, {}, {}, {}, 0, false, end_label, conditional->span});
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, else_label, conditional->span});
        if (conditional->else_branch) generate_statement(*conditional->else_branch);
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, end_label, conditional->span});
    } else if (const auto* loop = dynamic_cast<const ast::WhileStmt*>(&statement)) {
        // A loop label marks the condition check; true bodies jump back, false exits.
        const std::string start_label = new_label();
        const std::string end_label = new_label();
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, start_label, loop->span});
        const std::string condition = generate_expression(*loop->condition);
        program_.instructions.push_back({OpCode::JumpIfFalse, {}, condition, {}, 0, false, end_label, loop->condition->span});
        generate_statement(*loop->body);
        program_.instructions.push_back({OpCode::Jump, {}, {}, {}, 0, false, start_label, loop->span});
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, end_label, loop->span});
    }
}

std::string Generator::generate_expression(const ast::Expr& expression) {
    if (const auto* number = dynamic_cast<const ast::NumberExpr*>(&expression)) {
        const std::string temporary = new_temporary();
        program_.instructions.push_back({OpCode::LoadConst, temporary, {}, {}, number->value, false, {}, number->span});
        return temporary;
    }
    if (const auto* boolean = dynamic_cast<const ast::BooleanExpr*>(&expression)) {
        const std::string temporary = new_temporary();
        program_.instructions.push_back({OpCode::LoadBool, temporary, {}, {}, 0, boolean->value, {}, boolean->span});
        return temporary;
    }
    if (const auto* name = dynamic_cast<const ast::NameExpr*>(&expression)) {
        const std::string temporary = new_temporary();
        const std::string& storage_name = name->storage_name.empty() ? name->name : name->storage_name;
        program_.instructions.push_back({OpCode::LoadName, temporary, storage_name, {}, 0, false, {}, name->span});
        return temporary;
    }
    if (const auto* unary = dynamic_cast<const ast::UnaryExpr*>(&expression)) {
        const std::string operand = generate_expression(*unary->operand);
        const std::string temporary = new_temporary();
        const OpCode opcode = unary->op == "-" ? OpCode::Negate : OpCode::Not;
        program_.instructions.push_back({opcode, temporary, operand, {}, 0, false, {}, unary->span});
        return temporary;
    }
    const auto* binary = dynamic_cast<const ast::BinaryExpr*>(&expression);
    if (binary == nullptr) throw std::logic_error("unknown expression node");
    if (binary->op == "&&") {
        const std::string left = generate_expression(*binary->left);
        const std::string result = new_temporary();
        const std::string false_label = new_label();
        const std::string end_label = new_label();
        program_.instructions.push_back({OpCode::JumpIfFalse, {}, left, {}, 0, false, false_label, binary->left->span});

        const std::string right = generate_expression(*binary->right);
        program_.instructions.push_back({OpCode::JumpIfFalse, {}, right, {}, 0, false, false_label, binary->right->span});
        program_.instructions.push_back({OpCode::LoadBool, result, {}, {}, 0, true, {}, binary->span});
        program_.instructions.push_back({OpCode::Jump, {}, {}, {}, 0, false, end_label, binary->span});
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, false_label, binary->span});
        program_.instructions.push_back({OpCode::LoadBool, result, {}, {}, 0, false, {}, binary->span});
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, end_label, binary->span});
        return result;
    }
    if (binary->op == "||") {
        const std::string left = generate_expression(*binary->left);
        const std::string result = new_temporary();
        const std::string right_label = new_label();
        const std::string false_label = new_label();
        const std::string end_label = new_label();
        program_.instructions.push_back({OpCode::JumpIfFalse, {}, left, {}, 0, false, right_label, binary->left->span});
        program_.instructions.push_back({OpCode::LoadBool, result, {}, {}, 0, true, {}, binary->span});
        program_.instructions.push_back({OpCode::Jump, {}, {}, {}, 0, false, end_label, binary->span});
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, right_label, binary->span});

        const std::string right = generate_expression(*binary->right);
        program_.instructions.push_back({OpCode::JumpIfFalse, {}, right, {}, 0, false, false_label, binary->right->span});
        program_.instructions.push_back({OpCode::LoadBool, result, {}, {}, 0, true, {}, binary->span});
        program_.instructions.push_back({OpCode::Jump, {}, {}, {}, 0, false, end_label, binary->span});
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, false_label, binary->span});
        program_.instructions.push_back({OpCode::LoadBool, result, {}, {}, 0, false, {}, binary->span});
        program_.instructions.push_back({OpCode::Label, {}, {}, {}, 0, false, end_label, binary->span});
        return result;
    }
    const std::string left = generate_expression(*binary->left);
    const std::string right = generate_expression(*binary->right);
    const std::string temporary = new_temporary();
    OpCode opcode = OpCode::Add;
    if (binary->op == "+") opcode = OpCode::Add;
    else if (binary->op == "-") opcode = OpCode::Subtract;
    else if (binary->op == "*") opcode = OpCode::Multiply;
    else if (binary->op == "/") opcode = OpCode::Divide;
    else if (binary->op == "==") opcode = OpCode::Equal;
    else if (binary->op == "!=") opcode = OpCode::NotEqual;
    else if (binary->op == "<") opcode = OpCode::Less;
    else if (binary->op == "<=") opcode = OpCode::LessEqual;
    else if (binary->op == ">") opcode = OpCode::Greater;
    else if (binary->op == ">=") opcode = OpCode::GreaterEqual;
    else if (binary->op == "&&") opcode = OpCode::And;
    else if (binary->op == "||") opcode = OpCode::Or;
    program_.instructions.push_back({opcode, temporary, left, right, 0, false, {}, binary->span});
    return temporary;
}

std::string Generator::new_temporary() { return "t" + std::to_string(temporary_index_++); }
std::string Generator::new_label() { return "L" + std::to_string(label_index_++); }

}  // namespace edulang::ir
