#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "edulang/common/source.hpp"

namespace edulang::ir {

enum class OpCode {
    LoadConst,
    LoadBool,
    LoadName,
    StoreName,
    Add,
    Subtract,
    Multiply,
    Divide,
    Negate,
    Not,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    And,
    Or,
    Label,
    Jump,
    JumpIfFalse,
    Print,
};

struct Instruction {
    OpCode op{OpCode::LoadConst};
    std::string result{};
    std::string left{};
    std::string right{};
    std::int64_t value{0};
    bool boolean_value{false};
    std::string target{};
    SourceSpan span{};
};

struct Program {
    std::vector<Instruction> instructions;
};

[[nodiscard]] const char* opcode_name(OpCode opcode);
void print(const Program& program, std::ostream& output);

}  // namespace edulang::ir
