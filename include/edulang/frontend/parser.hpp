#pragma once

#include <vector>

#include "edulang/ast/ast.hpp"
#include "edulang/frontend/diagnostic.hpp"
#include "edulang/frontend/token.hpp"

namespace edulang {

class Parser {
public:
    Parser(const std::vector<Token>& tokens, DiagnosticBag& diagnostics);
    ast::Program parse();

private:
    ast::StmtPtr statement();
    ast::StmtPtr declaration();
    ast::StmtPtr assignment();
    ast::StmtPtr print_statement();
    ast::ExprPtr expression();
    ast::ExprPtr term();
    ast::ExprPtr factor();
    ast::ExprPtr primary();

    bool match(TokenType type);
    bool check(TokenType type) const;
    const Token& advance();
    const Token& peek() const;
    const Token& previous() const;
    bool at_end() const;
    void error(const Token& token, const char* message);
    void synchronize();

    const std::vector<Token>& tokens_;
    DiagnosticBag& diagnostics_;
    std::size_t current_{0};
};

}  