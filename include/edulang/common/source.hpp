#pragma once

#include <cstddef>

namespace edulang {

struct SourcePosition {
    std::size_t offset{0};
    std::size_t line{1};
    std::size_t column{1};
};

struct SourceSpan {
    SourcePosition start{};
    SourcePosition end{};
};

[[nodiscard]] inline SourceSpan combine_spans(const SourceSpan& first,
                                              const SourceSpan& last) {
    return SourceSpan{first.start, last.end};
}

}  
