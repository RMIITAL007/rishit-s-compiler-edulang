#include "edulang/target/code_generator.hpp"

namespace edulang::target {

void CodeGenerator::generate(const ir::Program& program, std::ostream& output) const {
    output << "; EduLang pseudo-assembly\n";
    for (const ir::Instruction& instruction : program.instructions) {
        switch (instruction.op) {
            case ir::OpCode::LoadConst: output << "MOV " << instruction.result << ", #" << instruction.value; break;
            case ir::OpCode::LoadBool: output << "MOV " << instruction.result << ", #" << (instruction.boolean_value ? 1 : 0); break;
            case ir::OpCode::LoadName: output << "MOV " << instruction.result << ", [" << instruction.left << "]"; break;
            case ir::OpCode::StoreName: output << "MOV [" << instruction.result << "], " << instruction.left; break;
            case ir::OpCode::Add: output << "ADD "; goto binary;
            case ir::OpCode::Subtract: output << "SUB "; goto binary;
            case ir::OpCode::Multiply: output << "MUL "; goto binary;
            case ir::OpCode::Divide: output << "DIV "; goto binary;
            case ir::OpCode::Equal: output << "CMPEQ "; goto binary;
            case ir::OpCode::NotEqual: output << "CMPNE "; goto binary;
            case ir::OpCode::Less: output << "CMPLT "; goto binary;
            case ir::OpCode::LessEqual: output << "CMPLE "; goto binary;
            case ir::OpCode::Greater: output << "CMPGT "; goto binary;
            case ir::OpCode::GreaterEqual: output << "CMPGE "; goto binary;
            case ir::OpCode::And: output << "AND "; goto binary;
            case ir::OpCode::Or: output << "OR "; goto binary;
            binary:
                output << instruction.result << ", " << instruction.left << ", " << instruction.right;
                break;
            case ir::OpCode::Negate: output << "NEG " << instruction.result << ", " << instruction.left; break;
            case ir::OpCode::Not: output << "NOT " << instruction.result << ", " << instruction.left; break;
            case ir::OpCode::Label: output << instruction.target << ':'; break;
            case ir::OpCode::Jump: output << "JMP " << instruction.target; break;
            case ir::OpCode::JumpIfFalse: output << "JZ " << instruction.left << ", " << instruction.target; break;
            case ir::OpCode::Print: output << "PRINT " << instruction.left; break;
        }
        output << '\n';
    }
}

}  // namespace edulang::target
