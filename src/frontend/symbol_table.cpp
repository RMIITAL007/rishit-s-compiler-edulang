#include "edulang/frontend/symbol_table.hpp"

namespace edulang {

SymbolTable::SymbolTable() : scopes_(1) {}

void SymbolTable::enter_scope() {
    scopes_.emplace_back();
}

void SymbolTable::exit_scope() {
    if (scopes_.size() > 1) {
        const std::size_t scope_depth = scopes_.size() - 1;
        for (SymbolTableEntry& entry : entries_) {
            if (entry.active && entry.scope_depth == scope_depth) entry.active = false;
        }
        scopes_.pop_back();
    }
}

bool SymbolTable::declare(const std::string& name, Symbol symbol) {
    auto& current_scope = scopes_.back();
    const auto [entry, inserted] = current_scope.emplace(name, std::move(symbol));
    if (inserted) {
        entries_.push_back(SymbolTableEntry{name, entry->second, scopes_.size() - 1, true});
    }
    return inserted;
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope) {
        const auto symbol = scope->find(name);
        if (symbol != scope->end()) {
            return &symbol->second;
        }
    }
    return nullptr;
}

bool SymbolTable::is_declared_in_current_scope(const std::string& name) const {
    const auto& current_scope = scopes_.back();
    return current_scope.find(name) != current_scope.end();
}

std::vector<std::pair<std::string, Symbol>> SymbolTable::current_scope_entries() const {
    std::vector<std::pair<std::string, Symbol>> entries;
    for (const auto& [name, symbol] : scopes_.back()) {
        entries.emplace_back(name, symbol);
    }
    return entries;
}

const std::vector<SymbolTableEntry>& SymbolTable::all_entries() const {
    return entries_;
}

}  // namespace edulang
