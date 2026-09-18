#include "edulang/frontend/diagnostic.hpp"

#include <algorithm>
#include <sstream>

namespace edulang {
namespace {

[[nodiscard]] const char* severity_name(DiagnosticSeverity severity) {
    return severity == DiagnosticSeverity::Error ? "error" : "warning";
}

[[nodiscard]] std::string source_line_at(std::string_view source, std::size_t line_number) {
    std::size_t line_start = 0;
    for (std::size_t current_line = 1; current_line < line_number; ++current_line) {
        const std::size_t newline = source.find('\n', line_start);
        if (newline == std::string_view::npos) {
            return {};
        }
        line_start = newline + 1;
    }

    const std::size_t line_end = source.find('\n', line_start);
    return std::string(source.substr(line_start, line_end - line_start));
}

}  

void DiagnosticBag::error(SourceSpan span, std::string message) {
    diagnostics_.push_back(Diagnostic{DiagnosticSeverity::Error, span, std::move(message)});
}

void DiagnosticBag::warning(SourceSpan span, std::string message) {
    diagnostics_.push_back(Diagnostic{DiagnosticSeverity::Warning, span, std::move(message)});
}

bool DiagnosticBag::has_errors() const {
    return std::any_of(diagnostics_.begin(), diagnostics_.end(), [](const Diagnostic& diagnostic) {
        return diagnostic.severity == DiagnosticSeverity::Error;
    });
}

bool DiagnosticBag::empty() const {
    return diagnostics_.empty();
}

const std::vector<Diagnostic>& DiagnosticBag::all() const {
    return diagnostics_;
}

std::string DiagnosticBag::render(std::string_view source, std::string_view source_name) const {
    std::ostringstream output;

    for (const Diagnostic& diagnostic : diagnostics_) {
        const SourcePosition& position = diagnostic.span.start;
        output << source_name << ':' << position.line << ':' << position.column << ": "
               << severity_name(diagnostic.severity) << ": " << diagnostic.message << '\n';

        const std::string line = source_line_at(source, position.line);
        if (!line.empty() || position.line == 1) {
            output << "  | " << line << '\n';
            output << "  | ";
            const std::size_t spaces = position.column > 0 ? position.column - 1 : 0;
            output << std::string(spaces, ' ') << '^' << '\n';
        }
    }

    return output.str();
}

} 