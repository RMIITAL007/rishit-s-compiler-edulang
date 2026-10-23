#pragma once

#include <cstdint>
#include <optional>
#include <string>

#if __has_include("edulang/common/source.hpp")
#include "edulang/common/source.hpp"
#else
#include "../common/source.hpp"
#endif

namespace edulang {

enum class TokenType {
    EndOfFile,
    Identifier,
    Number,
    KwPrint,
    KwInt,
    KwIf,
    KwElse,
    KwWhile,
    KwTrue,
    KwFalse,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Semicolon,
    Plus,
    Minus,
    Star,
    Slash,
    Equal,
    EqualEqual,
    Bang,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    AndAnd,
    OrOr,
};

struct Token {
    TokenType type{TokenType::EndOfFile};
    std::string lexeme{};
    SourceSpan span{};
    std::optional<std::int64_t> number_value{};
};

[[nodiscard]] const char* token_type_name(TokenType type);
}  