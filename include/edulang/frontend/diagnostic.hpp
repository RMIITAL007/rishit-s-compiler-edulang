#pragma once

#include <string>
#include <string_view>
#include <vector>

#if __has_include("edulang/common/source.hpp")
#include "edulang/common/source.hpp"
#else
#include "../common/source.hpp"
#endif

namespace edulang {

enum class DiagnosticSeverity {
    Error,
    Warning,
};

struct Diagnostic {
    DiagnosticSeverity severity{DiagnosticSeverity::Error};
    SourceSpan span{};
    std::string message{};
    std::string hint{};
};

class DiagnosticBag {
public:
    void error(SourceSpan span, std::string message, std::string hint = "");
    void warning(SourceSpan span, std::string message, std::string hint = "");

    [[nodiscard]] bool has_errors() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] const std::vector<Diagnostic>& all() const;

    [[nodiscard]] std::string render(std::string_view source,
                                     std::string_view source_name) const;

private:
    std::vector<Diagnostic> diagnostics_{};
};

}  
