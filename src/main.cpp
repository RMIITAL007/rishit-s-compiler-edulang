#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "edulang/ast/printer.hpp"
#include "edulang/frontend/frontend.hpp"
#include "edulang/frontend/lexer.hpp"
#include "edulang/frontend/parser.hpp"
#include "edulang/frontend/semantic_analyzer.hpp"
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

const char* value_type_name(edulang::ValueType type) {
    switch (type) {
        case edulang::ValueType::Int: return "int";
        case edulang::ValueType::Bool: return "bool";
        case edulang::ValueType::Error: return "error";
    }
    return "unknown";
}

void print_symbol_table(const edulang::SymbolTable& symbols) {
    auto entries = symbols.all_entries();
    std::sort(entries.begin(), entries.end(),
              [](const auto& left, const auto& right) {
                  if (left.scope_depth != right.scope_depth) return left.scope_depth < right.scope_depth;
                  if (left.symbol.declaration_span.start.line != right.symbol.declaration_span.start.line) {
                      return left.symbol.declaration_span.start.line < right.symbol.declaration_span.start.line;
                  }
                  if (left.symbol.declaration_span.start.column != right.symbol.declaration_span.start.column) {
                      return left.symbol.declaration_span.start.column < right.symbol.declaration_span.start.column;
                  }
                  return left.source_name < right.source_name;
              });
    std::cout << "Symbol Table\n";
    std::cout << "Source Name  Storage Name  Scope  Status        Type  Declared At\n";
    for (const auto& entry : entries) {
        const std::string scope = entry.scope_depth == 0
            ? "global"
            : "block " + std::to_string(entry.scope_depth);
        std::cout << entry.source_name << "  " << entry.symbol.storage_name << "  "
                  << scope << "  " << (entry.active ? "active" : "out of scope") << "  "
                  << value_type_name(entry.symbol.type) << "  "
                  << entry.symbol.declaration_span.start.line << ':'
                  << entry.symbol.declaration_span.start.column << '\n';
    }
}

void print_stage_status(const char* stage, bool passed) {
    std::cout << '[' << (passed ? "PASS" : "FAIL") << "] " << stage << '\n';
}

void print_skipped_stage(const char* stage) {
    std::cout << "[SKIP] " << stage << " (an earlier stage failed)\n";
}

bool run_benchmark(const std::string& source, const std::string& file_name) {
    using namespace std::chrono;
    std::cout << "========================================================\n";
    std::cout << "        EduLang Compiler Benchmark & Performance        \n";
    std::cout << "========================================================\n";
    std::cout << "Source File: " << file_name << " (" << source.size() << " bytes)\n\n";

    edulang::DiagnosticBag diagnostics;
    const auto report_diagnostics = [&] {
        std::cout.flush();
        std::cerr << diagnostics.render(source, file_name);
    };
    const auto total_start = high_resolution_clock::now();

    // 1. Lexical Analysis
    const auto lex_start = high_resolution_clock::now();
    edulang::Lexer lexer(source, diagnostics);
    const auto tokens = lexer.scan_tokens();
    const auto lex_end = high_resolution_clock::now();
    const auto lex_duration = duration_cast<nanoseconds>(lex_end - lex_start).count();

    if (diagnostics.has_errors()) {
        report_diagnostics();
        return false;
    }

    // 2. Syntax Analysis & AST Construction
    const auto parse_start = high_resolution_clock::now();
    edulang::Parser parser(tokens, diagnostics);
    auto program = parser.parse();
    const auto parse_end = high_resolution_clock::now();
    const auto parse_duration = duration_cast<nanoseconds>(parse_end - parse_start).count();

    if (diagnostics.has_errors()) {
        report_diagnostics();
        return false;
    }

    // 3. Semantic Analysis & Scoping
    const auto sem_start = high_resolution_clock::now();
    edulang::SymbolTable symbols;
    edulang::SemanticAnalyzer analyzer;
    analyzer.analyze(program, diagnostics, symbols);
    const auto sem_end = high_resolution_clock::now();
    const auto sem_duration = duration_cast<nanoseconds>(sem_end - sem_start).count();

    if (diagnostics.has_errors()) {
        report_diagnostics();
        return false;
    }

    // 4. IR Generation
    const auto ir_start = high_resolution_clock::now();
    edulang::ir::Generator generator;
    const auto unoptimized_ir = generator.generate(program);
    const auto ir_end = high_resolution_clock::now();
    const auto ir_duration = duration_cast<nanoseconds>(ir_end - ir_start).count();

    // 5. Optimization Pass
    const auto opt_start = high_resolution_clock::now();
    edulang::ir::Optimizer optimizer;
    const auto opt_result = optimizer.optimize_with_stats(unoptimized_ir);
    const auto opt_end = high_resolution_clock::now();
    const auto opt_duration = duration_cast<nanoseconds>(opt_end - opt_start).count();

    // 6. Target Code Generation
    const auto target_start = high_resolution_clock::now();
    std::ostringstream target_buffer;
    edulang::target::CodeGenerator target_gen;
    target_gen.generate(opt_result.program, target_buffer);
    const auto target_end = high_resolution_clock::now();
    const auto target_duration = duration_cast<nanoseconds>(target_end - target_start).count();

    // 7. Runtime Execution
    std::ostringstream runtime_buffer;
    const auto run_start = high_resolution_clock::now();
    edulang::runtime::Interpreter interpreter;
    const bool run_ok = interpreter.run(opt_result.program, diagnostics, runtime_buffer);
    const auto run_end = high_resolution_clock::now();
    const auto run_duration = duration_cast<nanoseconds>(run_end - run_start).count();

    const auto total_end = high_resolution_clock::now();
    const auto total_duration = duration_cast<nanoseconds>(total_end - total_start).count();

    std::cout << std::left << std::setw(32) << "Pipeline Stage" << std::right << std::setw(14) << "Time (us)" << std::setw(12) << "% Total" << '\n';
    std::cout << std::string(58, '-') << '\n';
    auto print_row = [&](const char* name, long long ns) {
        const double us = ns / 1000.0;
        const double pct = total_duration > 0 ? (100.0 * ns / total_duration) : 0.0;
        std::cout << std::left << std::setw(32) << name
                  << std::right << std::fixed << std::setprecision(2) << std::setw(14) << us
                  << std::setw(11) << pct << "%\n";
    };

    print_row("1. Lexical Analysis", lex_duration);
    print_row("2. Syntax Parsing & AST", parse_duration);
    print_row("3. Semantic Analysis & Scopes", sem_duration);
    print_row("4. TAC IR Generation", ir_duration);
    print_row("5. Optimization Pass", opt_duration);
    print_row("6. Target Code Generation", target_duration);
    print_row("7. Runtime Execution", run_duration);
    std::cout << std::string(58, '=') << '\n';
    print_row("Total Pipeline Execution", total_duration);

    std::cout << "\nOptimization Metrics:\n";
    std::cout << "  - Tokens Generated:             " << tokens.size() << '\n';
    std::cout << "  - Unoptimized TAC Instructions: " << opt_result.stats.instructions_before << '\n';
    std::cout << "  - Optimized TAC Instructions:   " << opt_result.stats.instructions_after << '\n';
    std::cout << "  - Constant Expressions Folded:  " << opt_result.stats.constants_folded << '\n';
    const double reduction = opt_result.stats.instructions_before > 0
        ? (100.0 * opt_result.stats.constants_folded / opt_result.stats.instructions_before)
        : 0.0;
    std::cout << "  - Optimization Reduction Rate:  " << std::fixed << std::setprecision(1) << reduction << "%\n";

    if (!run_ok || diagnostics.has_errors()) {
        report_diagnostics();
        return false;
    }

    std::cout << "\nProgram Execution Output:\n" << runtime_buffer.str();
    return true;
}

}  

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: compiler [tokens|ast|symbols|ir|run|target|bench] <source-file>\n";
        return 2;
    }

    std::string mode = "all";
    std::string file_name;
    if (argc == 2) {
        file_name = argv[1];
    } else {
        mode = argv[1];
        file_name = argv[2];
        if (mode != "tokens" && mode != "ast" && mode != "symbols" && mode != "ir" &&
            mode != "run" && mode != "target" && mode != "bench") {
            std::cerr << "Mode must be 'tokens', 'ast', 'symbols', 'ir', 'run', 'target', or 'bench'.\n";
            return 2;
        }
    }

    std::string source;
    if (!read_file(file_name, source)) {
        std::cerr << "Cannot open " << file_name << '\n';
        return 2;
    }

    if (mode == "bench") {
        return run_benchmark(source, file_name) ? 0 : 1;
    }

    edulang::DiagnosticBag diagnostics;
    edulang::Frontend frontend;
    edulang::FrontendResult result = frontend.analyze(source, diagnostics);

    if (mode == "all") {
        std::cout << "Pipeline Status\n";
        print_stage_status("Lexical analysis", result.lexical_analysis_passed);
        if (result.lexical_analysis_passed) {
            print_stage_status("Syntax analysis", result.syntax_analysis_passed);
        } else {
            print_skipped_stage("Syntax analysis");
        }
        if (result.syntax_analysis_passed) {
            print_stage_status("Semantic analysis", result.semantic_analysis_passed);
        } else {
            print_skipped_stage("Semantic analysis");
        }
    }

    if (mode == "all" || mode == "tokens") {
        std::cout << "Tokens\n";
        print_tokens(result.tokens);
    }
    if (!diagnostics.has_errors() && (mode == "all" || mode == "ast")) {
        std::cout << "AST\n";
        edulang::ast::print_ast(result.program, std::cout);
    }
    if (!diagnostics.has_errors() && (mode == "all" || mode == "symbols")) {
        print_symbol_table(result.symbols);
    }
    if (!diagnostics.has_errors() && (mode == "all" || mode == "ir" || mode == "run" || mode == "target")) {
        edulang::ir::Generator generator;
        const edulang::ir::Program unoptimized_ir = generator.generate(result.program);
        edulang::ir::Optimizer optimizer;
        const auto opt_result = optimizer.optimize_with_stats(unoptimized_ir);
        const auto& ir = opt_result.program;
        if (mode == "all") {
            std::cout << "[PASS] IR generation and optimization ("
                      << opt_result.stats.instructions_after << " instructions";
            if (opt_result.stats.constants_folded > 0) {
                std::cout << ", " << opt_result.stats.constants_folded << " constant(s) folded";
            }
            std::cout << ")\n";
        }
        if (mode == "all" || mode == "ir") {
            std::cout << "Intermediate Code\n";
            edulang::ir::print(ir, std::cout);
        }
        if (mode == "all" || mode == "target") {
            if (mode == "all") {
                print_stage_status("Target-code generation", true);
            }
            std::cout << "Target Code\n";
            edulang::target::CodeGenerator code_generator;
            code_generator.generate(ir, std::cout);
        }
        if (mode == "all" || mode == "run") {
            std::cout << "Output\n";
            edulang::runtime::Interpreter interpreter;
            const bool execution_passed = interpreter.run(ir, diagnostics, std::cout);
            if (mode == "all") {
                print_stage_status("Program execution", execution_passed);
            }
        }
    }
    if (!diagnostics.empty()) {
        std::cout.flush();
        std::cerr << diagnostics.render(source, file_name);
        return 1;
    }
    return 0;
}
