#pragma once

#include <ostream>

#include "edulang/frontend/diagnostic.hpp"
#include "edulang/ir/ir.hpp"

namespace edulang::runtime {

class Interpreter {
public:
    bool run(const ir::Program& program, DiagnosticBag& diagnostics, std::ostream& output) const;
};

}  // namespace edulang::runtime
