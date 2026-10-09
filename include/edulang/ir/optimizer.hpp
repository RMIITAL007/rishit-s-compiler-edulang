#pragma once

#include "edulang/ir/ir.hpp"

namespace edulang::ir {

class Optimizer {
public:
    [[nodiscard]] Program optimize(const Program& program) const;
};

}  // namespace edulang::ir
