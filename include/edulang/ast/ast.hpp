#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "edulang/common/source.hpp"

namespace edulang::ast {

struct Expr {
    explicit Expr(SourceSpan span = {}) : span(span) {}
    SourceSpan span{};
    virtual ~Expr() = default;
};

using ExprPtr = std::unique_ptr<Expr>;

struct NumberExpr : Expr {
    NumberExpr(std::int64_t value, SourceSpan span = {}) : value(value) {
        this->span = span;
    }
    std::int64_t value;
};

struct BooleanExpr : Expr {
    BooleanExpr(bool value, SourceSpan span = {}) : value(value) {
        this->span = span;
    }
    bool value;
};

struct NameExpr : Expr {
    NameExpr(std::string name, SourceSpan span = {}) : name(std::move(name)) {
        this->span = span;
    }
    std::string name;
    std::string storage_name{};
};

struct BinaryExpr : Expr {
    BinaryExpr(ExprPtr left, std::string op, ExprPtr right, SourceSpan span = {})
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {
        this->span = span;
    }

    ExprPtr left;
    std::string op;
    ExprPtr right;
};

struct UnaryExpr : Expr {
    UnaryExpr(std::string op, ExprPtr operand, SourceSpan span = {})
        : op(std::move(op)), operand(std::move(operand)) {
        this->span = span;
    }
    std::string op;
    ExprPtr operand;
};

struct Stmt {
    explicit Stmt(SourceSpan span = {}) : span(span) {}
    SourceSpan span{};
    virtual ~Stmt() = default;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct DeclarationStmt : Stmt {
    DeclarationStmt(std::string name, ExprPtr value, SourceSpan span = {}, SourceSpan name_span = {})
        : Stmt{span}, name(std::move(name)), value(std::move(value)), name_span(name_span) {}

    std::string name;
    std::string storage_name{};
    ExprPtr value;
    SourceSpan name_span{};
};

struct AssignmentStmt : Stmt {
    AssignmentStmt(std::string name, ExprPtr value, SourceSpan span = {}, SourceSpan name_span = {})
        : Stmt{span}, name(std::move(name)), value(std::move(value)), name_span(name_span) {}

    std::string name;
    std::string storage_name{};
    ExprPtr value;
    SourceSpan name_span{};
};

struct PrintStmt : Stmt {
    explicit PrintStmt(ExprPtr value, SourceSpan span = {}) : Stmt{span}, value(std::move(value)) {}
    ExprPtr value;
};

struct BlockStmt : Stmt {
    explicit BlockStmt(std::vector<StmtPtr> statements = {}, SourceSpan span = {})
        : Stmt{span}, statements(std::move(statements)) {}
    std::vector<StmtPtr> statements;
};

struct IfStmt : Stmt {
    IfStmt(ExprPtr condition, StmtPtr then_branch, StmtPtr else_branch = nullptr,
           SourceSpan span = {})
        : Stmt{span}, condition(std::move(condition)), then_branch(std::move(then_branch)),
          else_branch(std::move(else_branch)) {}
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;
};

struct WhileStmt : Stmt {
    WhileStmt(ExprPtr condition, StmtPtr body, SourceSpan span = {})
        : Stmt{span}, condition(std::move(condition)), body(std::move(body)) {}
    ExprPtr condition;
    StmtPtr body;
};

struct Program {
    std::vector<StmtPtr> statements;
};

}  // namespace edulang::ast
