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

void DiagnosticBag::error(SourceSpan span, std::string message, std::string hint) {
    diagnostics_.push_back(Diagnostic{DiagnosticSeverity::Error, span, std::move(message), std::move(hint)});
}

void DiagnosticBag::warning(SourceSpan span, std::string message, std::string hint) {
    diagnostics_.push_back(Diagnostic{DiagnosticSeverity::Warning, span, std::move(message), std::move(hint)});
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
        const SourcePosition& start = diagnostic.span.start;
        const SourcePosition& end = diagnostic.span.end;
        output << source_name << ':' << start.line << ':' << start.column << ": "
               << severity_name(diagnostic.severity) << ": " << diagnostic.message << '\n';

        const std::string line = source_line_at(source, start.line);
        if (!line.empty() || start.line == 1) {
            output << "  | " << line << '\n';
            output << "  | ";

            const std::size_t start_column = start.column > 0 ? start.column - 1 : 0;
            const std::size_t end_column = end.column > 0 ? end.column - 1 : 0;
            const std::size_t width = (start.line == end.line && end_column > start_column)
                                         ? (end_column - start_column)
                                         : 1;
            output << std::string(start_column, ' ') << std::string(width, '^') << '\n';
        }

        if (!diagnostic.hint.empty()) {
            output << "  = hint: " << diagnostic.hint << '\n';
        }
    }

    return output.str();
}

} 
