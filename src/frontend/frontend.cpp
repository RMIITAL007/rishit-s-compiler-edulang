#include "edulang/frontend/frontend.hpp"

#include <utility>

#include "edulang/frontend/lexer.hpp"
#include "edulang/frontend/parser.hpp"

namespace edulang {

FrontendResult Frontend::analyze(std::string_view source, DiagnosticBag& diagnostics) const {
    Lexer lexer(source, diagnostics);
    std::vector<Token> tokens = lexer.scan_tokens();

    Parser parser(tokens, diagnostics);
    ast::Program program = parser.parse();
    return FrontendResult{std::move(tokens), std::move(program)};
}

}  
