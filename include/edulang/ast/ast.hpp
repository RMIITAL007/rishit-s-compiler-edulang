#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace edulang::ast {

struct Expr {
    virtual ~Expr() = default;
};

using ExprPtr = std::unique_ptr<Expr>;

struct NumberExpr : Expr {
    explicit NumberExpr(std::int64_t value) : value(value) {}
    std::int64_t value;
};

struct NameExpr : Expr {
    explicit NameExpr(std::string name) : name(std::move(name)) {}
    std::string name;
};

struct BinaryExpr : Expr {
    BinaryExpr(ExprPtr left, std::string op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    ExprPtr left;
    std::string op;
    ExprPtr right;
};

struct Stmt {
    virtual ~Stmt() = default;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct DeclarationStmt : Stmt {
    DeclarationStmt(std::string name, ExprPtr value)
        : name(std::move(name)), value(std::move(value)) {}

    std::string name;
    ExprPtr value;
};

struct AssignmentStmt : Stmt {
    AssignmentStmt(std::string name, ExprPtr value)
        : name(std::move(name)), value(std::move(value)) {}

    std::string name;
    ExprPtr value;
};

struct PrintStmt : Stmt {
    explicit PrintStmt(ExprPtr value) : value(std::move(value)) {}
    ExprPtr value;
};

struct Program {
    std::vector<StmtPtr> statements;
};

}  // namespace edulang::ast
