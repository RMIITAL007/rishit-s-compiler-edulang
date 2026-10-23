#pragma once

#include <cstddef>
#include <string>
#include <utility>
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
    std::string storage_name{};
};

struct SymbolTableEntry {
    std::string source_name{};
    Symbol symbol{};
    std::size_t scope_depth{0};
    bool active{true};
};

class SymbolTable {
public:
    SymbolTable();

    void enter_scope();
    void exit_scope();

    [[nodiscard]] bool declare(const std::string& name, Symbol symbol);
    [[nodiscard]] const Symbol* lookup(const std::string& name) const;
    [[nodiscard]] bool is_declared_in_current_scope(const std::string& name) const;
    [[nodiscard]] std::vector<std::pair<std::string, Symbol>> current_scope_entries() const;
    [[nodiscard]] const std::vector<SymbolTableEntry>& all_entries() const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    std::vector<SymbolTableEntry> entries_;
};

}  // namespace edulang
