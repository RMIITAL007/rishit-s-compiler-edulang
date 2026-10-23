#if __has_include("edulang/frontend/lexer.hpp")
#include "edulang/frontend/lexer.hpp"
#else
#include "../../include/edulang/frontend/lexer.hpp"
#endif

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace edulang {
namespace {

[[nodiscard]] TokenType keyword_or_identifier(std::string_view lexeme) {
    if (lexeme == "print") {
        return TokenType::KwPrint;
    }
    if (lexeme == "int") {
        return TokenType::KwInt;
    }
    if (lexeme == "if") return TokenType::KwIf;
    if (lexeme == "else") return TokenType::KwElse;
    if (lexeme == "while") return TokenType::KwWhile;
    if (lexeme == "true") return TokenType::KwTrue;
    if (lexeme == "false") return TokenType::KwFalse;
    return TokenType::Identifier;
}

}  
const char* token_type_name(TokenType type) {
    switch (type) {
        case TokenType::EndOfFile: return "end of file";
        case TokenType::Identifier: return "identifier";
        case TokenType::Number: return "number";
        case TokenType::KwPrint: return "'print'";
        case TokenType::KwInt: return "'int'";
        case TokenType::KwIf: return "'if'";
        case TokenType::KwElse: return "'else'";
        case TokenType::KwWhile: return "'while'";
        case TokenType::KwTrue: return "'true'";
        case TokenType::KwFalse: return "'false'";
        case TokenType::LeftParen: return "'('";
        case TokenType::RightParen: return "')'";
        case TokenType::LeftBrace: return "'{'";
        case TokenType::RightBrace: return "'}'";
        case TokenType::Semicolon: return "';'";
        case TokenType::Plus: return "'+'";
        case TokenType::Minus: return "'-'";
        case TokenType::Star: return "'*'";
        case TokenType::Slash: return "'/'";
        case TokenType::Equal: return "'='";
        case TokenType::EqualEqual: return "'=='";
        case TokenType::Bang: return "'!'";
        case TokenType::BangEqual: return "'!='";
        case TokenType::Less: return "'<'";
        case TokenType::LessEqual: return "'<='";
        case TokenType::Greater: return "'>'";
        case TokenType::GreaterEqual: return "'>='";
        case TokenType::AndAnd: return "'&&'";
        case TokenType::OrOr: return "'||'";
    }
    return "unknown token";
}

Lexer::Lexer(std::string_view source, DiagnosticBag& diagnostics)
    : source_(source), diagnostics_(diagnostics) {}

std::vector<Token> Lexer::scan_tokens() {
    while (!at_end()) {
        start_ = current_;
        start_position_ = position_;
        scan_token();
    }

    tokens_.push_back(Token{TokenType::EndOfFile, "", SourceSpan{position_, position_}, std::nullopt});
    return tokens_;
}

bool Lexer::at_end() const {
    return current_ >= source_.size();
}

char Lexer::advance() {
    const char character = source_[current_++];
    ++position_.offset;
    if (character == '\n') {
        ++position_.line;
        position_.column = 1;
    } else {
        ++position_.column;
    }
    return character;
}

char Lexer::peek() const {
    return at_end() ? '\0' : source_[current_];
}

char Lexer::peek_next() const {
    return current_ + 1 >= source_.size() ? '\0' : source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (at_end() || source_[current_] != expected) {
        return false;
    }
    static_cast<void>(advance());
    return true;
}

void Lexer::scan_token() {
    const char character = advance();
    switch (character) {
        case ' ':
        case '\r':
        case '\t':
        case '\n':
            return;
        case '(':
            add_token(TokenType::LeftParen);
            return;
        case ')':
            add_token(TokenType::RightParen);
            return;
        case '{':
            add_token(TokenType::LeftBrace);
            return;
        case '}':
            add_token(TokenType::RightBrace);
            return;
        case ';':
            add_token(TokenType::Semicolon);
            return;
        case '+':
            add_token(TokenType::Plus);
            return;
        case '-':
            add_token(TokenType::Minus);
            return;
        case '*':
            add_token(TokenType::Star);
            return;
        case '=':
            add_token(match('=') ? TokenType::EqualEqual : TokenType::Equal);
            return;
        case '!':
            add_token(match('=') ? TokenType::BangEqual : TokenType::Bang);
            return;
        case '<':
            add_token(match('=') ? TokenType::LessEqual : TokenType::Less);
            return;
        case '>':
            add_token(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
            return;
        case '&':
            if (match('&')) add_token(TokenType::AndAnd);
            else diagnostics_.error(SourceSpan{start_position_, position_}, "expected '&' after '&'");
            return;
        case '|':
            if (match('|')) add_token(TokenType::OrOr);
            else diagnostics_.error(SourceSpan{start_position_, position_}, "expected '|' after '|'");
            return;
        case '/':
            if (match('/')) {
                while (peek() != '\n' && !at_end()) {
                    static_cast<void>(advance());
                }
            } else if (match('*')) {
                block_comment();
            } else {
                add_token(TokenType::Slash);
            }
            return;
        default:
            if (is_digit(character)) {
                number();
            } else if (is_alpha(character)) {
                identifier();
            } else {
                diagnostics_.error(SourceSpan{start_position_, position_},
                                   std::string("unexpected character '") + character + "'");
            }
            return;
    }
}

void Lexer::identifier() {
    while (is_alpha_numeric(peek())) {
        static_cast<void>(advance());
    }
    add_token(keyword_or_identifier(source_.substr(start_, current_ - start_)));
}

void Lexer::number() {
    while (is_digit(peek())) {
        static_cast<void>(advance());
    }

    const std::string_view lexeme = source_.substr(start_, current_ - start_);
    std::int64_t value = 0;
    const auto [end, error] = std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), value);
    if (error != std::errc{} || end != lexeme.data() + lexeme.size()) {
        diagnostics_.error(SourceSpan{start_position_, position_}, "integer literal is out of range");
        add_token(TokenType::Number);
        return;
    }
    add_number_token(value);
}

void Lexer::block_comment() {
    while (!at_end()) {
        if (peek() == '*' && peek_next() == '/') {
            static_cast<void>(advance());
            static_cast<void>(advance());
            return;
        }
        static_cast<void>(advance());
    }

    diagnostics_.error(SourceSpan{start_position_, position_}, "unterminated block comment");
}

void Lexer::add_token(TokenType type) {
    tokens_.push_back(Token{type, std::string(source_.substr(start_, current_ - start_)),
                            SourceSpan{start_position_, position_}, std::nullopt});
}

void Lexer::add_number_token(std::int64_t value) {
    tokens_.push_back(Token{TokenType::Number,
                            std::string(source_.substr(start_, current_ - start_)),
                            SourceSpan{start_position_, position_}, value});
}

bool Lexer::is_digit(char character) {
    return character >= '0' && character <= '9';
}

bool Lexer::is_alpha(char character) {
    return (character >= 'a' && character <= 'z') ||
           (character >= 'A' && character <= 'Z') || character == '_';
}

bool Lexer::is_alpha_numeric(char character) {
    return is_alpha(character) || is_digit(character);
}

}  