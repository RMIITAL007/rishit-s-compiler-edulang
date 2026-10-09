#include "edulang/frontend/frontend.hpp"

#include <utility>

#include "edulang/frontend/lexer.hpp"
#include "edulang/frontend/parser.hpp"
#include "edulang/frontend/semantic_analyzer.hpp"

namespace edulang {

FrontendResult Frontend::analyze(std::string_view source, DiagnosticBag& diagnostics) const {
    Lexer lexer(source, diagnostics);
    std::vector<Token> tokens = lexer.scan_tokens();

    Parser parser(tokens, diagnostics);
    ast::Program program = parser.parse();
    SymbolTable symbols;

    if (!diagnostics.has_errors()) {
        SemanticAnalyzer semantic_analyzer;
        semantic_analyzer.analyze(program, diagnostics, symbols);
    }

    return FrontendResult{std::move(tokens), std::move(program), std::move(symbols)};
}

}  
