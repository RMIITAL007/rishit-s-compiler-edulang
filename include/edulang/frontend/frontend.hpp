#pragma once

#include <string_view>
#include <vector>

#include "edulang/ast/ast.hpp"
#include "edulang/frontend/diagnostic.hpp"
#include "edulang/frontend/symbol_table.hpp"
#include "edulang/frontend/token.hpp"

namespace edulang {

struct FrontendResult {
    std::vector<Token> tokens;
    ast::Program program;
    SymbolTable symbols;
    bool lexical_analysis_passed{false};
    bool syntax_analysis_passed{false};
    bool semantic_analysis_passed{false};
};

class Frontend {
public:
    [[nodiscard]] FrontendResult analyze(std::string_view source, DiagnosticBag& diagnostics) const;
};

}  
