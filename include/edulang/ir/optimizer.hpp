#pragma once

#include "edulang/ir/ir.hpp"

namespace edulang::ir {

struct OptimizationStats {
    std::size_t instructions_before{0};
    std::size_t instructions_after{0};
    std::size_t constants_folded{0};
};

struct OptimizedProgram {
    Program program;
    OptimizationStats stats;
};

class Optimizer {
public:
    [[nodiscard]] Program optimize(const Program& program) const;
    [[nodiscard]] OptimizedProgram optimize_with_stats(const Program& program) const;
};

}  // namespace edulang::ir
