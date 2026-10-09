#pragma once

#include <ostream>

#include "edulang/ir/ir.hpp"

namespace edulang::target {

class CodeGenerator {
public:
    void generate(const ir::Program& program, std::ostream& output) const;
};

}  // namespace edulang::target
