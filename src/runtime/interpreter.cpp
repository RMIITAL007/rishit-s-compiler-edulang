#include "edulang/runtime/interpreter.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace edulang::runtime {
namespace {

struct Value {
    bool is_bool{false};
    std::int64_t integer{0};
    bool boolean{false};
};

bool read_value(const std::string& name, const std::unordered_map<std::string, Value>& values, Value& value) {
    const auto found = values.find(name);
    if (found == values.end()) return false;
    value = found->second;
    return true;
}

bool require_int(const Value& value, std::int64_t& result) {
    if (value.is_bool) return false;
    result = value.integer;
    return true;
}

bool require_bool(const Value& value, bool& result) {
    if (!value.is_bool) return false;
    result = value.boolean;
    return true;
}

}  // namespace

bool Interpreter::run(const ir::Program& program, DiagnosticBag& diagnostics, std::ostream& output) const {
    std::unordered_map<std::string, Value> values;
    std::unordered_map<std::string, std::size_t> labels;
    for (std::size_t index = 0; index < program.instructions.size(); ++index) {
        if (program.instructions[index].op == ir::OpCode::Label) labels[program.instructions[index].target] = index;
    }

    std::size_t pc = 0;
    while (pc < program.instructions.size()) {
        const ir::Instruction& instruction = program.instructions[pc];
        Value left_value;
        Value right_value;
        std::int64_t left = 0;
        std::int64_t right = 0;
        bool left_bool = false;
        bool right_bool = false;
        switch (instruction.op) {
            case ir::OpCode::LoadConst:
                values[instruction.result] = Value{false, instruction.value, false}; ++pc; break;
            case ir::OpCode::LoadBool:
                values[instruction.result] = Value{true, 0, instruction.boolean_value}; ++pc; break;
            case ir::OpCode::LoadName:
                if (!read_value(instruction.left, values, left_value)) {
                    diagnostics.error(instruction.span, "runtime variable '" + instruction.left + "' has no value"); return false;
                }
                values[instruction.result] = left_value; ++pc; break;
            case ir::OpCode::StoreName:
                if (!read_value(instruction.left, values, left_value)) {
                    diagnostics.error(instruction.span, "runtime value '" + instruction.left + "' is unavailable"); return false;
                }
                values[instruction.result] = left_value; ++pc; break;
            case ir::OpCode::Negate:
                if (!read_value(instruction.left, values, left_value) || !require_int(left_value, left)) {
                    diagnostics.error(instruction.span, "runtime negation requires int"); return false;
                }
                values[instruction.result] = Value{false, -left, false}; ++pc; break;
            case ir::OpCode::Not:
                if (!read_value(instruction.left, values, left_value) || !require_bool(left_value, left_bool)) {
                    diagnostics.error(instruction.span, "runtime not requires bool"); return false;
                }
                values[instruction.result] = Value{true, 0, !left_bool}; ++pc; break;
            case ir::OpCode::Add:
            case ir::OpCode::Subtract:
            case ir::OpCode::Multiply:
            case ir::OpCode::Divide:
            case ir::OpCode::Less:
            case ir::OpCode::LessEqual:
            case ir::OpCode::Greater:
            case ir::OpCode::GreaterEqual:
                if (!read_value(instruction.left, values, left_value) || !read_value(instruction.right, values, right_value) ||
                    !require_int(left_value, left) || !require_int(right_value, right)) {
                    diagnostics.error(instruction.span, "runtime arithmetic or comparison requires int operands"); return false;
                }
                if (instruction.op == ir::OpCode::Divide && right == 0) {
                    diagnostics.error(instruction.span, "division by zero"); return false;
                }
                if (instruction.op == ir::OpCode::Add) values[instruction.result] = Value{false, left + right, false};
                if (instruction.op == ir::OpCode::Subtract) values[instruction.result] = Value{false, left - right, false};
                if (instruction.op == ir::OpCode::Multiply) values[instruction.result] = Value{false, left * right, false};
                if (instruction.op == ir::OpCode::Divide) values[instruction.result] = Value{false, left / right, false};
                if (instruction.op == ir::OpCode::Less) values[instruction.result] = Value{true, 0, left < right};
                if (instruction.op == ir::OpCode::LessEqual) values[instruction.result] = Value{true, 0, left <= right};
                if (instruction.op == ir::OpCode::Greater) values[instruction.result] = Value{true, 0, left > right};
                if (instruction.op == ir::OpCode::GreaterEqual) values[instruction.result] = Value{true, 0, left >= right};
                ++pc; break;
            case ir::OpCode::Equal: {
            case ir::OpCode::NotEqual:
                if (!read_value(instruction.left, values, left_value) || !read_value(instruction.right, values, right_value) || left_value.is_bool != right_value.is_bool) {
                    diagnostics.error(instruction.span, "runtime equality operands have different types"); return false;
                }
                const bool equal = left_value.is_bool ? left_value.boolean == right_value.boolean : left_value.integer == right_value.integer;
                values[instruction.result] = Value{true, 0, instruction.op == ir::OpCode::Equal ? equal : !equal}; ++pc; break;
            }
            case ir::OpCode::And:
            case ir::OpCode::Or:
                if (!read_value(instruction.left, values, left_value) || !read_value(instruction.right, values, right_value) ||
                    !require_bool(left_value, left_bool) || !require_bool(right_value, right_bool)) {
                    diagnostics.error(instruction.span, "runtime logical operator requires bool operands"); return false;
                }
                values[instruction.result] = Value{true, 0, instruction.op == ir::OpCode::And ? left_bool && right_bool : left_bool || right_bool}; ++pc; break;
            case ir::OpCode::Label: ++pc; break;
            case ir::OpCode::Jump:
                pc = labels[instruction.target]; break;
            case ir::OpCode::JumpIfFalse:
                if (!read_value(instruction.left, values, left_value) || !require_bool(left_value, left_bool)) {
                    diagnostics.error(instruction.span, "conditional jump requires bool"); return false;
                }
                pc = left_bool ? pc + 1 : labels[instruction.target]; break;
            case ir::OpCode::Print:
                if (!read_value(instruction.left, values, left_value)) {
                    diagnostics.error(instruction.span, "runtime print value is unavailable"); return false;
                }
                if (left_value.is_bool) output << (left_value.boolean ? "true" : "false") << '\n';
                else output << left_value.integer << '\n';
                ++pc; break;
        }
    }
    return true;
}

}  // namespace edulang::runtime
