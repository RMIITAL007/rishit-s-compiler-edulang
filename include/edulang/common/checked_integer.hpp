#pragma once

#include <cstdint>
#include <limits>

namespace edulang::checked_integer {

inline bool add(std::int64_t left, std::int64_t right, std::int64_t& result) {
    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    if ((right > 0 && left > maximum - right) ||
        (right < 0 && left < minimum - right)) {
        return false;
    }
    result = left + right;
    return true;
}

inline bool subtract(std::int64_t left, std::int64_t right, std::int64_t& result) {
    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    if ((right > 0 && left < minimum + right) ||
        (right < 0 && left > maximum + right)) {
        return false;
    }
    result = left - right;
    return true;
}

inline bool multiply(std::int64_t left, std::int64_t right, std::int64_t& result) {
    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
    if (left == 0 || right == 0) {
        result = 0;
        return true;
    }
    if ((left > 0 && right > 0 && left > maximum / right) ||
        (left > 0 && right < 0 && right < minimum / left) ||
        (left < 0 && right > 0 && left < minimum / right) ||
        (left < 0 && right < 0 && left < maximum / right)) {
        return false;
    }
    result = left * right;
    return true;
}

inline bool divide(std::int64_t left, std::int64_t right, std::int64_t& result) {
    constexpr auto minimum = std::numeric_limits<std::int64_t>::min();
    if (right == 0 || (left == minimum && right == -1)) {
        return false;
    }
    result = left / right;
    return true;
}

inline bool negate(std::int64_t value, std::int64_t& result) {
    if (value == std::numeric_limits<std::int64_t>::min()) {
        return false;
    }
    result = -value;
    return true;
}

}  // namespace edulang::checked_integer
