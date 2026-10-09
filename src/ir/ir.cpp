#include "edulang/ir/ir.hpp"

#include <ostream>

namespace edulang::ir {

const char* opcode_name(OpCode opcode) {
    switch (opcode) {
        case OpCode::LoadConst: return "load_const";
        case OpCode::LoadBool: return "load_bool";
        case OpCode::LoadName: return "load_name";
        case OpCode::StoreName: return "store_name";
        case OpCode::Add: return "add";
        case OpCode::Subtract: return "sub";
        case OpCode::Multiply: return "mul";
        case OpCode::Divide: return "div";
        case OpCode::Negate: return "neg";
        case OpCode::Not: return "not";
        case OpCode::Equal: return "eq";
        case OpCode::NotEqual: return "neq";
        case OpCode::Less: return "lt";
        case OpCode::LessEqual: return "lte";
        case OpCode::Greater: return "gt";
        case OpCode::GreaterEqual: return "gte";
        case OpCode::And: return "and";
        case OpCode::Or: return "or";
        case OpCode::Label: return "label";
        case OpCode::Jump: return "jump";
        case OpCode::JumpIfFalse: return "jump_if_false";
        case OpCode::Print: return "print";
    }
    return "unknown";
}

void print(const Program& program, std::ostream& output) {
    for (std::size_t index = 0; index < program.instructions.size(); ++index) {
        const Instruction& instruction = program.instructions[index];
        output << index << ": " << opcode_name(instruction.op);
        switch (instruction.op) {
            case OpCode::LoadConst: output << " " << instruction.result << ", " << instruction.value; break;
            case OpCode::LoadBool: output << " " << instruction.result << ", " << (instruction.boolean_value ? "true" : "false"); break;
            case OpCode::LoadName: output << " " << instruction.result << ", " << instruction.left; break;
            case OpCode::StoreName: output << " " << instruction.result << ", " << instruction.left; break;
            case OpCode::Negate:
            case OpCode::Not: output << " " << instruction.result << ", " << instruction.left; break;
            case OpCode::Add:
            case OpCode::Subtract:
            case OpCode::Multiply:
            case OpCode::Divide:
            case OpCode::Equal:
            case OpCode::NotEqual:
            case OpCode::Less:
            case OpCode::LessEqual:
            case OpCode::Greater:
            case OpCode::GreaterEqual:
            case OpCode::And:
            case OpCode::Or:
                output << " " << instruction.result << ", " << instruction.left << ", " << instruction.right;
                break;
            case OpCode::Label:
            case OpCode::Jump: output << " " << instruction.target; break;
            case OpCode::JumpIfFalse: output << " " << instruction.left << ", " << instruction.target; break;
            case OpCode::Print: output << " " << instruction.left; break;
        }
        output << '\n';
    }
}

}  // namespace edulang::ir
