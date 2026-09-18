#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "edulang/common/source.hpp"

namespace edulang {

enum class DiagnosticSeverity {
    Error,
    Warning,
};

struct Diagnostic {
    DiagnosticSeverity severity{DiagnosticSeverity::Error};
    SourceSpan span{};
    std::string message{};
};

class DiagnosticBag {
public:
    void error(SourceSpan span, std::string message);
    void warning(SourceSpan span, std::string message);

    [[nodiscard]] bool has_errors() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] const std::vector<Diagnostic>& all() const;

    [[nodiscard]] std::string render(std::string_view source,
                                     std::string_view source_name) const;

private:
    std::vector<Diagnostic> diagnostics_{};
};

}  
