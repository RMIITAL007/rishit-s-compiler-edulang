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
        if (ast::StmtPtr parsed = statement()) {
            program.statements.push_back(std::move(parsed));
        }
        if (current_ == before && !at_end()) {
            advance();
        }
    }
    return program;
}

ast::StmtPtr Parser::statement() {
    if (match(TokenType::KwInt)) {
        return declaration();
    }
    if (match(TokenType::KwPrint)) {
        return print_statement();
    }
    if (check(TokenType::Identifier)) {
        return assignment();
    }
    error(peek(), "expected 'int', 'print', or an assignment");
    synchronize();
    return nullptr;
}

ast::StmtPtr Parser::declaration() {
    if (!match(TokenType::Identifier)) {
        error(peek(), "expected a variable name after 'int'");
        synchronize();
        return nullptr;
    }
    const std::string name = previous().lexeme;
    if (!match(TokenType::Equal)) {
        error(peek(), "expected '=' after variable name");
        synchronize();
        return nullptr;
    }
    ast::ExprPtr value = expression();
    if (!value || !match(TokenType::Semicolon)) {
        if (value) {
            error(peek(), "expected ';' after declaration");
        }
        synchronize();
        return nullptr;
    }
    return std::make_unique<ast::DeclarationStmt>(name, std::move(value));
}

ast::StmtPtr Parser::assignment() {
    const std::string name = advance().lexeme;
    if (!match(TokenType::Equal)) {
        error(peek(), "expected '=' after variable name");
        synchronize();
        return nullptr;
    }
    ast::ExprPtr value = expression();
    if (!value || !match(TokenType::Semicolon)) {
        if (value) {
            error(peek(), "expected ';' after assignment");
        }
        synchronize();
        return nullptr;
    }
    return std::make_unique<ast::AssignmentStmt>(name, std::move(value));
}

ast::StmtPtr Parser::print_statement() {
    if (!match(TokenType::LeftParen)) {
        error(peek(), "expected '(' after 'print'");
        synchronize();
        return nullptr;
    }
    ast::ExprPtr value = expression();
    if (!value || !match(TokenType::RightParen)) {
        if (value) {
            error(peek(), "expected ')' after print expression");
        }
        synchronize();
        return nullptr;
    }
    if (!match(TokenType::Semicolon)) {
        error(peek(), "expected ';' after print statement");
        synchronize();
        return nullptr;
    }
    return std::make_unique<ast::PrintStmt>(std::move(value));
}

ast::ExprPtr Parser::expression() {
    ast::ExprPtr left = term();
    while (left && (match(TokenType::Plus) || match(TokenType::Minus))) {
        const std::string op = previous().lexeme;
        ast::ExprPtr right = term();
        if (!right) {
            return nullptr;
        }
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ast::ExprPtr Parser::term() {
    ast::ExprPtr left = factor();
    while (left && (match(TokenType::Star) || match(TokenType::Slash))) {
        const std::string op = previous().lexeme;
        ast::ExprPtr right = factor();
        if (!right) {
            return nullptr;
        }
        left = std::make_unique<ast::BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ast::ExprPtr Parser::factor() {
    if (match(TokenType::Number)) {
        return std::make_unique<ast::NumberExpr>(previous().number_value.value_or(0));
    }
    if (match(TokenType::Identifier)) {
        return std::make_unique<ast::NameExpr>(previous().lexeme);
    }
    if (match(TokenType::LeftParen)) {
        ast::ExprPtr value = expression();
        if (!match(TokenType::RightParen)) {
            error(peek(), "expected ')' after expression");
            return nullptr;
        }
        return value;
    }
    error(peek(), "expected a number, variable name, or '('");
    return nullptr;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

const Token& Parser::advance() {
    if (!at_end()) {
        ++current_;
    }
    return previous();
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

bool Parser::at_end() const {
    return peek().type == TokenType::EndOfFile;
}

void Parser::error(const Token& token, const char* message) {
    diagnostics_.error(token.span, message);
}

void Parser::synchronize() {
    while (!at_end() && !match(TokenType::Semicolon)) {
        advance();
    }
}

} 