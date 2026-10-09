#pragma once

#include "edulang/ast/ast.hpp"
#include "edulang/ir/ir.hpp"

namespace edulang::ir {

class Generator {
public:
    [[nodiscard]] Program generate(const ast::Program& program);

private:
    [[nodiscard]] std::string generate_expression(const ast::Expr& expression);
    std::string new_temporary();
    std::string new_label();
    void generate_statement(const ast::Stmt& statement);

    Program program_;
    std::size_t temporary_index_{0};
    std::size_t label_index_{0};
};

}  // namespace edulang::ir
