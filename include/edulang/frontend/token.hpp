#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "edulang/common/source.hpp"

namespace edulang {

enum class TokenType {
    EndOfFile,
    Identifier,
    Number,
    KwPrint,
    KwInt,
    LeftParen,
    RightParen,
    Semicolon,
    Plus,
    Minus,
    Star,
    Slash,
    Equal,
};

struct Token {
    TokenType type{TokenType::EndOfFile};
    std::string lexeme{};
    SourceSpan span{};
    std::optional<std::int64_t> number_value{};
};

[[nodiscard]] const char* token_type_name(TokenType type);
}  