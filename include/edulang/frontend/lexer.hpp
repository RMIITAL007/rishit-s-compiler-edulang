#pragma once

#include <string_view>
#include <vector>

#include "edulang/frontend/diagnostic.hpp"
#include "edulang/frontend/token.hpp"

namespace edulang {

class Lexer {
public:
    Lexer(std::string_view source, DiagnosticBag& diagnostics);

    [[nodiscard]] std::vector<Token> scan_tokens();

private:
    [[nodiscard]] bool at_end() const;
    char advance();
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peek_next() const;
    bool match(char expected);

    void scan_token();
    void identifier();
    void number();
    void block_comment();
    void add_token(TokenType type);
    void add_number_token(std::int64_t value);

    [[nodiscard]] static bool is_digit(char character);
    [[nodiscard]] static bool is_alpha(char character);
    [[nodiscard]] static bool is_alpha_numeric(char character);

    std::string_view source_;
    DiagnosticBag& diagnostics_;
    std::vector<Token> tokens_{};
    std::size_t start_{0};
    std::size_t current_{0};
    SourcePosition start_position_{};
    SourcePosition position_{};
};

}  
