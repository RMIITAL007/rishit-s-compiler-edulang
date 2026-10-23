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
    ast::StmtPtr if_statement();
    ast::StmtPtr while_statement();
    ast::StmtPtr block_statement();
    ast::ExprPtr expression();
    ast::ExprPtr or_expression();
    ast::ExprPtr and_expression();
    ast::ExprPtr equality();
    ast::ExprPtr comparison();
    ast::ExprPtr additive();
    ast::ExprPtr term();
    ast::ExprPtr factor();
    ast::ExprPtr primary();

    bool match(TokenType type);
    bool check(TokenType type) const;
    const Token& advance();
    const Token& peek() const;
    const Token& previous() const;
    bool at_end() const;
    bool is_statement_start() const;
    void error(const Token& token, const char* message, const char* hint = "");
    void synchronize();

    const std::vector<Token>& tokens_;
    DiagnosticBag& diagnostics_;
    std::size_t current_{0};
};

}  // namespace edulang
