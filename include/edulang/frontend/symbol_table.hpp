#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "edulang/common/source.hpp"

namespace edulang {

enum class ValueType {
    Int,
    Bool,
    Error,
};

struct Symbol {
    ValueType type{ValueType::Int};
    SourceSpan declaration_span{};
};

class SymbolTable {
public:
    SymbolTable();

    void enter_scope();
    void exit_scope();

    [[nodiscard]] bool declare(const std::string& name, Symbol symbol);
    [[nodiscard]] const Symbol* lookup(const std::string& name) const;
    [[nodiscard]] bool is_declared_in_current_scope(const std::string& name) const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
};

}  // namespace edulang
