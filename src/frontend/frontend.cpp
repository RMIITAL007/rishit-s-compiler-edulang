#include "edulang/frontend/frontend.hpp"

#include <utility>

#include "edulang/frontend/lexer.hpp"
#include "edulang/frontend/parser.hpp"
#include "edulang/frontend/semantic_analyzer.hpp"

namespace edulang {

FrontendResult Frontend::analyze(std::string_view source, DiagnosticBag& diagnostics) const {
    Lexer lexer(source, diagnostics);
    std::vector<Token> tokens = lexer.scan_tokens();
    const bool lexical_analysis_passed = !diagnostics.has_errors();

    ast::Program program;
    SymbolTable symbols;
    bool syntax_analysis_passed = false;
    bool semantic_analysis_passed = false;

    if (lexical_analysis_passed) {
        Parser parser(tokens, diagnostics);
        program = parser.parse();
        syntax_analysis_passed = !diagnostics.has_errors();
    }

    if (syntax_analysis_passed) {
        SemanticAnalyzer semantic_analyzer;
        semantic_analyzer.analyze(program, diagnostics, symbols);
        semantic_analysis_passed = !diagnostics.has_errors();
    }

    return FrontendResult{std::move(tokens),
                          std::move(program),
                          std::move(symbols),
                          lexical_analysis_passed,
                          syntax_analysis_passed,
                          semantic_analysis_passed};
}

}  
