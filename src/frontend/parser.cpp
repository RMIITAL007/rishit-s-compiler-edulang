#include "edulang/frontend/parser.hpp"

#include <memory>
#include <utility>

namespace edulang {

Parser::Parser(const std::vector<Token>& tokens, DiagnosticBag& diagnostics)
    : tokens_(tokens), diagnostics_(diagnostics) {}

ast::Program Parser::parse() {
    ast::Program program;
    while (!at_end()) {
        const std::size_t before = current_;
        if (ast::StmtPtr parsed = statement()) program.statements.push_back(std::move(parsed));
        if (current_ == before && !at_end()) advance();
    }
    return program;
}

ast::StmtPtr Parser::statement() {
    if (match(TokenType::KwInt)) return declaration();
    if (match(TokenType::KwPrint)) return print_statement();
    if (match(TokenType::KwIf)) return if_statement();
    if (match(TokenType::KwWhile)) return while_statement();
    if (match(TokenType::LeftBrace)) return block_statement();
    if (check(TokenType::Identifier)) return assignment();
    error(peek(), "expected 'int', 'print', 'if', 'while', or an assignment");
    synchronize();
    return nullptr;
}

ast::StmtPtr Parser::declaration() {
    const auto start = previous().span;
    if (!match(TokenType::Identifier)) { error(peek(), "expected a variable name after 'int'"); synchronize(); return nullptr; }
    const std::string name = previous().lexeme;
    const auto name_span = previous().span;
    if (!match(TokenType::Equal)) { error(peek(), "expected '=' after variable name"); synchronize(); return nullptr; }
    ast::ExprPtr value = expression();
    if (!value || !match(TokenType::Semicolon)) { if (value) error(peek(), "expected ';' after declaration"); synchronize(); return nullptr; }
    return std::make_unique<ast::DeclarationStmt>(name, std::move(value), combine_spans(start, previous().span), name_span);
}

ast::StmtPtr Parser::assignment() {
    const auto start = peek().span;
    const std::string name = advance().lexeme;
    const auto name_span = previous().span;
    if (!match(TokenType::Equal)) { error(peek(), "expected '=' after variable name"); synchronize(); return nullptr; }
    ast::ExprPtr value = expression();
    if (!value || !match(TokenType::Semicolon)) { if (value) error(peek(), "expected ';' after assignment"); synchronize(); return nullptr; }
    return std::make_unique<ast::AssignmentStmt>(name, std::move(value), combine_spans(start, previous().span), name_span);
}

ast::StmtPtr Parser::print_statement() {
    const auto start = previous().span;
    if (!match(TokenType::LeftParen)) { error(peek(), "expected '(' after 'print'"); synchronize(); return nullptr; }
    ast::ExprPtr value = expression();
    if (!value || !match(TokenType::RightParen)) { if (value) error(peek(), "expected ')' after print expression"); synchronize(); return nullptr; }
    if (!match(TokenType::Semicolon)) { error(peek(), "expected ';' after print statement"); synchronize(); return nullptr; }
    return std::make_unique<ast::PrintStmt>(std::move(value), combine_spans(start, previous().span));
}

ast::StmtPtr Parser::if_statement() {
    const auto start = previous().span;
    if (!match(TokenType::LeftParen)) { error(peek(), "expected '(' after 'if'"); synchronize(); return nullptr; }
    ast::ExprPtr condition = expression();
    if (!match(TokenType::RightParen)) { error(peek(), "expected ')' after if condition"); synchronize(); return nullptr; }
    ast::StmtPtr then_branch = statement();
    if (!then_branch) return nullptr;
    ast::StmtPtr else_branch;
    if (match(TokenType::KwElse)) else_branch = statement();
    const auto end = else_branch ? else_branch->span : then_branch->span;
    return std::make_unique<ast::IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch), combine_spans(start, end));
}

ast::StmtPtr Parser::while_statement() {
    const auto start = previous().span;
    if (!match(TokenType::LeftParen)) { error(peek(), "expected '(' after 'while'"); synchronize(); return nullptr; }
    ast::ExprPtr condition = expression();
    if (!match(TokenType::RightParen)) { error(peek(), "expected ')' after while condition"); synchronize(); return nullptr; }
    ast::StmtPtr body = statement();
    if (!body) return nullptr;
    const auto end = body->span;
    return std::make_unique<ast::WhileStmt>(std::move(condition), std::move(body), combine_spans(start, end));
}

ast::StmtPtr Parser::block_statement() {
    const auto start = previous().span;
    std::vector<ast::StmtPtr> statements;
    while (!check(TokenType::RightBrace) && !at_end()) {
        if (ast::StmtPtr parsed = statement()) statements.push_back(std::move(parsed));
    }
    if (!match(TokenType::RightBrace)) { error(peek(), "expected '}' after block"); return nullptr; }
    return std::make_unique<ast::BlockStmt>(std::move(statements), combine_spans(start, previous().span));
}

// Precedence is layered from lowest to highest: ||, &&, equality, comparison,
// addition/subtraction, multiplication/division, then unary and primary values.
ast::ExprPtr Parser::expression() { return or_expression(); }

ast::ExprPtr Parser::or_expression() {
    ast::ExprPtr left = and_expression();
    while (left && match(TokenType::OrOr)) {
        const std::string op = previous().lexeme; ast::ExprPtr right = and_expression(); if (!right) return nullptr;
        const auto span = combine_spans(left->span, right->span);
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right), span);
    }
    return left;
}

ast::ExprPtr Parser::and_expression() {
    ast::ExprPtr left = equality();
    while (left && match(TokenType::AndAnd)) {
        const std::string op = previous().lexeme; ast::ExprPtr right = equality(); if (!right) return nullptr;
        const auto span = combine_spans(left->span, right->span);
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right), span);
    }
    return left;
}

ast::ExprPtr Parser::equality() {
    ast::ExprPtr left = comparison();
    while (left && (match(TokenType::EqualEqual) || match(TokenType::BangEqual))) {
        const std::string op = previous().lexeme; ast::ExprPtr right = comparison(); if (!right) return nullptr;
        const auto span = combine_spans(left->span, right->span);
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right), span);
    }
    return left;
}

ast::ExprPtr Parser::comparison() {
    ast::ExprPtr left = additive();
    while (left && (match(TokenType::Less) || match(TokenType::LessEqual) || match(TokenType::Greater) || match(TokenType::GreaterEqual))) {
        const std::string op = previous().lexeme; ast::ExprPtr right = additive(); if (!right) return nullptr;
        const auto span = combine_spans(left->span, right->span);
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right), span);
    }
    return left;
}

ast::ExprPtr Parser::additive() {
    ast::ExprPtr left = term();
    while (left && (match(TokenType::Plus) || match(TokenType::Minus))) {
        const std::string op = previous().lexeme; ast::ExprPtr right = term(); if (!right) return nullptr;
        const auto span = combine_spans(left->span, right->span);
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right), span);
    }
    return left;
}

ast::ExprPtr Parser::term() {
    ast::ExprPtr left = factor();
    while (left && (match(TokenType::Star) || match(TokenType::Slash))) {
        const std::string op = previous().lexeme; ast::ExprPtr right = factor(); if (!right) return nullptr;
        const auto span = combine_spans(left->span, right->span);
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right), span);
    }
    return left;
}

ast::ExprPtr Parser::factor() {
    if (match(TokenType::Bang) || match(TokenType::Minus)) {
        const auto start = previous().span; const std::string op = previous().lexeme; ast::ExprPtr operand = factor();
        if (!operand) return nullptr;
        const auto span = combine_spans(start, operand->span);
        return std::make_unique<ast::UnaryExpr>(op, std::move(operand), span);
    }
    return primary();
}

ast::ExprPtr Parser::primary() {
    if (match(TokenType::Number)) return std::make_unique<ast::NumberExpr>(previous().number_value.value_or(0), previous().span);
    if (match(TokenType::KwTrue)) return std::make_unique<ast::BooleanExpr>(true, previous().span);
    if (match(TokenType::KwFalse)) return std::make_unique<ast::BooleanExpr>(false, previous().span);
    if (match(TokenType::Identifier)) return std::make_unique<ast::NameExpr>(previous().lexeme, previous().span);
    if (match(TokenType::LeftParen)) {
        const auto start = previous().span; ast::ExprPtr value = expression();
        if (!match(TokenType::RightParen)) { error(peek(), "expected ')' after expression"); return nullptr; }
        if (value) value->span = combine_spans(start, previous().span);
        return value;
    }
    error(peek(), "expected a number, boolean, variable name, or '('"); return nullptr;
}

bool Parser::match(TokenType type) { if (!check(type)) return false; advance(); return true; }
bool Parser::check(TokenType type) const { return peek().type == type; }
const Token& Parser::advance() { if (!at_end()) ++current_; return previous(); }
const Token& Parser::peek() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }
bool Parser::at_end() const { return peek().type == TokenType::EndOfFile; }
void Parser::error(const Token& token, const char* message) { diagnostics_.error(token.span, message); }
void Parser::synchronize() { while (!at_end() && !match(TokenType::Semicolon) && !check(TokenType::RightBrace)) advance(); }

}  // namespace edulang
