#pragma once

#include "edulang/ast/ast.hpp"
#include "edulang/frontend/diagnostic.hpp"
#include "edulang/frontend/symbol_table.hpp"

namespace edulang {

class SemanticAnalyzer {
public:
    void analyze(ast::Program& program, DiagnosticBag& diagnostics, SymbolTable& symbols) const;
};

}  // namespace edulang
