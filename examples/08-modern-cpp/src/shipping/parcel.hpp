// A parcel and the Weighable concept that constrains the code operating on it.
// Header-only: the concept and the constrained function are both compile-time
// machinery, so there is nothing to put in a .cpp.
#ifndef SHIPPING_PARCEL_HPP
#define SHIPPING_PARCEL_HPP

#include <concepts>

namespace shipping {

// A plain value type: a parcel knows its weight and whether it is fragile.
struct Parcel {
  int weight_grams = 0;
  bool fragile = false;
};

// A type is Weighable if it exposes an integer weight in grams. The requires
// expression names the one property the code below depends on, so any type that
// provides it, not only Parcel, satisfies the constraint.
template <typename T>
concept Weighable = requires(const T& item) {
  { item.weight_grams } -> std::convertible_to<int>;
};

// The heavier of two weighable items, by weight. Constraining the template on
// Weighable means a call with an unsuitable type is rejected at the call site
// with a readable message, not deep inside the body.
template <Weighable T>
constexpr int heavier_weight(const T& first, const T& second) {
  return first.weight_grams >= second.weight_grams ? first.weight_grams
                                                   : second.weight_grams;
}

}  // namespace shipping

#endif  // SHIPPING_PARCEL_HPP
