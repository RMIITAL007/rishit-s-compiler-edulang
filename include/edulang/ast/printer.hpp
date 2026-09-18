#pragma once

#include <ostream>

#include "edulang/ast/ast.hpp"

namespace edulang::ast {

void print_ast(const Program& program, std::ostream& output);

}  
