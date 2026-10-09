#pragma once

#include "edulang/ast/ast.hpp"
#include "edulang/frontend/diagnostic.hpp"

namespace edulang {

class SemanticAnalyzer {
public:
    void analyze(const ast::Program& program, DiagnosticBag& diagnostics) const;
};

}  // namespace edulang
