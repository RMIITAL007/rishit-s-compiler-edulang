#include "edulang/ir/optimizer.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace edulang::ir {
namespace {

struct Constant {
    bool is_bool{false};
    std::int64_t integer{0};
    bool boolean{false};
};

bool binary_constant(OpCode opcode, const Constant& left, const Constant& right, Constant& result) {
    if (opcode == OpCode::And || opcode == OpCode::Or) {
        if (!left.is_bool || !right.is_bool) return false;
        result = {true, 0, opcode == OpCode::And ? left.boolean && right.boolean : left.boolean || right.boolean};
        return true;
    }
    if (opcode == OpCode::Equal || opcode == OpCode::NotEqual) {
        if (left.is_bool != right.is_bool) return false;
        const bool equal = left.is_bool ? left.boolean == right.boolean : left.integer == right.integer;
        result = {true, 0, opcode == OpCode::Equal ? equal : !equal};
        return true;
    }
    if (left.is_bool || right.is_bool) return false;
    if (opcode == OpCode::Divide && right.integer == 0) return false;
    if (opcode == OpCode::Add) result = {false, left.integer + right.integer, false};
    else if (opcode == OpCode::Subtract) result = {false, left.integer - right.integer, false};
    else if (opcode == OpCode::Multiply) result = {false, left.integer * right.integer, false};
    else if (opcode == OpCode::Divide) result = {false, left.integer / right.integer, false};
    else if (opcode == OpCode::Less) result = {true, 0, left.integer < right.integer};
    else if (opcode == OpCode::LessEqual) result = {true, 0, left.integer <= right.integer};
    else if (opcode == OpCode::Greater) result = {true, 0, left.integer > right.integer};
    else if (opcode == OpCode::GreaterEqual) result = {true, 0, left.integer >= right.integer};
    else return false;
    return true;
}

}  // namespace

Program Optimizer::optimize(const Program& program) const {
    Program optimized;
    std::unordered_map<std::string, Constant> constants;
    for (const Instruction& instruction : program.instructions) {
        Instruction output = instruction;
        if (instruction.op == OpCode::LoadConst) {
            constants[instruction.result] = {false, instruction.value, false};
        } else if (instruction.op == OpCode::LoadBool) {
            constants[instruction.result] = {true, 0, instruction.boolean_value};
        } else if (instruction.op == OpCode::Negate || instruction.op == OpCode::Not) {
            const auto found = constants.find(instruction.left);
            if (found != constants.end()) {
                if (instruction.op == OpCode::Negate && !found->second.is_bool) {
                    output = {OpCode::LoadConst, instruction.result, {}, {}, -found->second.integer, false, {}, instruction.span};
                    constants[instruction.result] = {false, -found->second.integer, false};
                } else if (instruction.op == OpCode::Not && found->second.is_bool) {
                    output = {OpCode::LoadBool, instruction.result, {}, {}, 0, !found->second.boolean, {}, instruction.span};
                    constants[instruction.result] = {true, 0, !found->second.boolean};
                }
            }
        } else if (instruction.op == OpCode::Add || instruction.op == OpCode::Subtract || instruction.op == OpCode::Multiply ||
                   instruction.op == OpCode::Divide || instruction.op == OpCode::Equal || instruction.op == OpCode::NotEqual ||
                   instruction.op == OpCode::Less || instruction.op == OpCode::LessEqual || instruction.op == OpCode::Greater ||
                   instruction.op == OpCode::GreaterEqual || instruction.op == OpCode::And || instruction.op == OpCode::Or) {
            const auto left = constants.find(instruction.left);
            const auto right = constants.find(instruction.right);
            Constant result;
            if (left != constants.end() && right != constants.end() && binary_constant(instruction.op, left->second, right->second, result)) {
                output = result.is_bool
                    ? Instruction{OpCode::LoadBool, instruction.result, {}, {}, 0, result.boolean, {}, instruction.span}
                    : Instruction{OpCode::LoadConst, instruction.result, {}, {}, result.integer, false, {}, instruction.span};
                constants[instruction.result] = result;
            } else {
                constants.erase(instruction.result);
            }
        } else if (instruction.op == OpCode::LoadName || instruction.op == OpCode::StoreName ||
                   instruction.op == OpCode::Label || instruction.op == OpCode::Jump || instruction.op == OpCode::JumpIfFalse) {
            constants.clear();
        }
        optimized.instructions.push_back(output);
    }
    return optimized;
}

}  // namespace edulang::ir
