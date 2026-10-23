#include "edulang/ir/optimizer.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

#include "edulang/common/checked_integer.hpp"

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
    std::int64_t integer_result = 0;
    if (opcode == OpCode::Add && checked_integer::add(left.integer, right.integer, integer_result)) {
        result = {false, integer_result, false};
    } else if (opcode == OpCode::Subtract && checked_integer::subtract(left.integer, right.integer, integer_result)) {
        result = {false, integer_result, false};
    } else if (opcode == OpCode::Multiply && checked_integer::multiply(left.integer, right.integer, integer_result)) {
        result = {false, integer_result, false};
    } else if (opcode == OpCode::Divide && checked_integer::divide(left.integer, right.integer, integer_result)) {
        result = {false, integer_result, false};
    }
    else if (opcode == OpCode::Less) result = {true, 0, left.integer < right.integer};
    else if (opcode == OpCode::LessEqual) result = {true, 0, left.integer <= right.integer};
    else if (opcode == OpCode::Greater) result = {true, 0, left.integer > right.integer};
    else if (opcode == OpCode::GreaterEqual) result = {true, 0, left.integer >= right.integer};
    else return false;
    return true;
}

}  // namespace

OptimizedProgram Optimizer::optimize_with_stats(const Program& program) const {
    OptimizedProgram result;
    result.stats.instructions_before = program.instructions.size();

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
                    std::int64_t negated = 0;
                    if (checked_integer::negate(found->second.integer, negated)) {
                        output = {OpCode::LoadConst, instruction.result, {}, {}, negated, false, {}, instruction.span};
                        constants[instruction.result] = {false, negated, false};
                        ++result.stats.constants_folded;
                    } else {
                        constants.erase(instruction.result);
                    }
                } else if (instruction.op == OpCode::Not && found->second.is_bool) {
                    output = {OpCode::LoadBool, instruction.result, {}, {}, 0, !found->second.boolean, {}, instruction.span};
                    constants[instruction.result] = {true, 0, !found->second.boolean};
                    ++result.stats.constants_folded;
                } else {
                    constants.erase(instruction.result);
                }
            } else {
                constants.erase(instruction.result);
            }
        } else if (instruction.op == OpCode::Add || instruction.op == OpCode::Subtract || instruction.op == OpCode::Multiply ||
                   instruction.op == OpCode::Divide || instruction.op == OpCode::Equal || instruction.op == OpCode::NotEqual ||
                   instruction.op == OpCode::Less || instruction.op == OpCode::LessEqual || instruction.op == OpCode::Greater ||
                   instruction.op == OpCode::GreaterEqual || instruction.op == OpCode::And || instruction.op == OpCode::Or) {
            const auto left = constants.find(instruction.left);
            const auto right = constants.find(instruction.right);
            Constant folded;
            if (left != constants.end() && right != constants.end() && binary_constant(instruction.op, left->second, right->second, folded)) {
                output = folded.is_bool
                    ? Instruction{OpCode::LoadBool, instruction.result, {}, {}, 0, folded.boolean, {}, instruction.span}
                    : Instruction{OpCode::LoadConst, instruction.result, {}, {}, folded.integer, false, {}, instruction.span};
                constants[instruction.result] = folded;
                ++result.stats.constants_folded;
            } else {
                constants.erase(instruction.result);
            }
        } else if (instruction.op == OpCode::LoadName || instruction.op == OpCode::StoreName ||
                   instruction.op == OpCode::Label || instruction.op == OpCode::Jump || instruction.op == OpCode::JumpIfFalse) {
            constants.clear();
        }
        result.program.instructions.push_back(output);
    }

    result.stats.instructions_after = result.program.instructions.size();
    return result;
}

Program Optimizer::optimize(const Program& program) const {
    return optimize_with_stats(program).program;
}

}  // namespace edulang::ir
