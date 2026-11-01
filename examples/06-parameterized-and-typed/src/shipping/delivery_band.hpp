#ifndef SHIPPING_DELIVERY_BAND_HPP
#define SHIPPING_DELIVERY_BAND_HPP

#include <cstdint>

namespace shipping {

// The band a parcel ships under, decided by weight.
enum class Band : std::uint8_t { Letter, Parcel, Freight };

// Classify a parcel by weight: up to 500 g is a Letter, up to 30000 g a Parcel,
// anything heavier is Freight. `weight_grams` must be positive, or the call
// throws std::invalid_argument.
[[nodiscard]] Band delivery_band(int weight_grams);

}  // namespace shipping

#endif  // SHIPPING_DELIVERY_BAND_HPP
