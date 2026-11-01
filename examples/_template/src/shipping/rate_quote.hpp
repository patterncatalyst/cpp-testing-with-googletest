#ifndef SHIPPING_RATE_QUOTE_HPP
#define SHIPPING_RATE_QUOTE_HPP

#include <cstdint>

namespace shipping {

// The delivery zone a parcel is shipped to. Farther zones cost more.
enum class Zone : std::uint8_t { Domestic, Regional, International };

// Quote the shipping price, in cents, for a parcel of the given weight.
// Weight is billed per whole kilogram, rounded up. `weight_grams` must be
// positive; a non-positive weight throws std::invalid_argument.
[[nodiscard]] int quote_cents(int weight_grams, Zone zone);

}  // namespace shipping

#endif  // SHIPPING_RATE_QUOTE_HPP
