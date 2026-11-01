#include "shipping/delivery_band.hpp"

#include <stdexcept>

namespace shipping {

Band delivery_band(int weight_grams) {
  if (weight_grams <= 0) {
    throw std::invalid_argument("weight_grams must be positive");
  }
  if (weight_grams <= 500) {
    return Band::Letter;
  }
  if (weight_grams <= 30000) {
    return Band::Parcel;
  }
  return Band::Freight;
}

}  // namespace shipping
