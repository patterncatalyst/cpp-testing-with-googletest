#ifndef SHIPPING_CLAMP_HPP
#define SHIPPING_CLAMP_HPP

namespace shipping {

// Clamp a measurement to non-negative: a negative value becomes zero, anything
// else is returned unchanged. Works for any arithmetic type, so the same rule
// covers a count, a weight in grams, and a distance as a double.
template <typename T>
[[nodiscard]] T clamp_non_negative(T value) {
  return value < T{0} ? T{0} : value;
}

}  // namespace shipping

#endif  // SHIPPING_CLAMP_HPP
