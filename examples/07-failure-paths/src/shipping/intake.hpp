#ifndef SHIPPING_INTAKE_HPP
#define SHIPPING_INTAKE_HPP

#include <cstdint>
#include <expected>
#include <string_view>

namespace shipping {

// Why a parsed weight was rejected. A recoverable outcome a caller can handle,
// so it travels as a value rather than an exception.
enum class ParseError : std::uint8_t { Empty, NotANumber, NotPositive };

// Parse a weight in grams from text. Returns the grams on success, or a
// ParseError on failure. The whole text must be a positive integer.
[[nodiscard]] std::expected<int, ParseError> parse_weight(std::string_view text);

// The same parse, but a rejection throws std::invalid_argument with a message
// naming the reason, for callers that prefer exceptions to a result type.
[[nodiscard]] int parse_weight_or_throw(std::string_view text);

// Remaining capacity on a truck, in grams. `used` must not exceed `total`;
// breaking that is a programming error, checked with assert, which aborts the
// process in a build with assertions enabled.
[[nodiscard]] int remaining_capacity(int total_grams, int used_grams);

}  // namespace shipping

#endif  // SHIPPING_INTAKE_HPP
