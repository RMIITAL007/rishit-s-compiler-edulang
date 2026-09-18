#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "edulang/ast/printer.hpp"
#include "edulang/frontend/frontend.hpp"

namespace {

bool read_file(const std::string& path, std::string& source) {
    std::ifstream input(path);
    if (!input) {
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    source = buffer.str();
    return true;
}

void print_tokens(const std::vector<edulang::Token>& tokens) {
    for (const edulang::Token& token : tokens) {
        std::cout << token.span.start.line << ':' << token.span.start.column << "  "
                  << edulang::token_type_name(token.type) << "  ";
        if (token.type == edulang::TokenType::EndOfFile) {
            std::cout << "EOF";
        } else {
            std::cout << std::quoted(token.lexeme);
        }
        std::cout << '\n';
    }
}

}  

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: compiler [tokens|ast] <source-file>\n";
        return 2;
    }

    std::string mode = "all";
    std::string file_name;
    if (argc == 2) {
        file_name = argv[1];
    } else {
        mode = argv[1];
        file_name = argv[2];
        if (mode != "tokens" && mode != "ast") {
            std::cerr << "Mode must be 'tokens' or 'ast'.\n";
            return 2;
        }
    }

    std::string source;
    if (!read_file(file_name, source)) {
        std::cerr << "Cannot open " << file_name << '\n';
        return 2;
    }

    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    edulang::FrontendResult result = frontend.analyze(source, diagnostics);

    if (mode == "all" || mode == "tokens") {
        std::cout << "Tokens\n";
        print_tokens(result.tokens);
    }
    if (!diagnostics.has_errors() && (mode == "all" || mode == "ast")) {
        std::cout << "AST\n";
        edulang::ast::print_ast(result.program, std::cout);
    }
    if (!diagnostics.empty()) {
        std::cerr << diagnostics.render(source, file_name);
        return 1;
    }
    return 0;
}
