#include "shipping/intake.hpp"

#include <cassert>
#include <charconv>
#include <stdexcept>

namespace shipping {

std::expected<int, ParseError> parse_weight(std::string_view text) {
  if (text.empty()) {
    return std::unexpected(ParseError::Empty);
  }

  int value = 0;
  const char* const end = text.data() + text.size();
  const auto [ptr, ec] = std::from_chars(text.data(), end, value);
  if (ec != std::errc{} || ptr != end) {
    return std::unexpected(ParseError::NotANumber);
  }
  if (value <= 0) {
    return std::unexpected(ParseError::NotPositive);
  }
  return value;
}

int parse_weight_or_throw(std::string_view text) {
  const std::expected<int, ParseError> result = parse_weight(text);
  if (result) {
    return *result;
  }
  switch (result.error()) {
    case ParseError::Empty:
      throw std::invalid_argument("weight text is empty");
    case ParseError::NotANumber:
      throw std::invalid_argument("weight text is not a number");
    case ParseError::NotPositive:
      throw std::invalid_argument("weight must be positive");
  }
  throw std::invalid_argument("unknown parse error");  // unreachable
}

int remaining_capacity(int total_grams, int used_grams) {
  assert(used_grams <= total_grams && "used must not exceed total");
  return total_grams - used_grams;
}

}  // namespace shipping
