#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "edulang/ast/printer.hpp"
#include "edulang/frontend/frontend.hpp"
#include "edulang/ir/generator.hpp"
#include "edulang/ir/ir.hpp"
#include "edulang/ir/optimizer.hpp"
#include "edulang/runtime/interpreter.hpp"
#include "edulang/target/code_generator.hpp"

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
        std::cerr << "Usage: compiler [tokens|ast|ir|run|target] <source-file>\n";
        return 2;
    }

    std::string mode = "all";
    std::string file_name;
    if (argc == 2) {
        file_name = argv[1];
    } else {
        mode = argv[1];
        file_name = argv[2];
        if (mode != "tokens" && mode != "ast" && mode != "ir" && mode != "run" && mode != "target") {
            std::cerr << "Mode must be 'tokens', 'ast', 'ir', 'run', or 'target'.\n";
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
    if (!diagnostics.has_errors() && (mode == "all" || mode == "ir" || mode == "run" || mode == "target")) {
        edulang::ir::Generator generator;
        const edulang::ir::Program unoptimized_ir = generator.generate(result.program);
        edulang::ir::Optimizer optimizer;
        const edulang::ir::Program ir = optimizer.optimize(unoptimized_ir);
        if (mode == "all" || mode == "ir") {
            std::cout << "Intermediate Code\n";
            edulang::ir::print(ir, std::cout);
        }
        if (mode == "all" || mode == "target") {
            std::cout << "Target Code\n";
            edulang::target::CodeGenerator code_generator;
            code_generator.generate(ir, std::cout);
        }
        if (mode == "all" || mode == "run") {
            std::cout << "Output\n";
            edulang::runtime::Interpreter interpreter;
            interpreter.run(ir, diagnostics, std::cout);
        }
    }
    if (!diagnostics.empty()) {
        std::cerr << diagnostics.render(source, file_name);
        return 1;
    }
    return 0;
}
