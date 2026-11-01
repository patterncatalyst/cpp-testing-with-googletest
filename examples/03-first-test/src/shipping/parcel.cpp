#include "shipping/parcel.hpp"

#include <algorithm>

namespace shipping {

int billable_weight_grams(Dimensions box, int actual_grams) {
  const int volumetric_grams =
      (box.length_cm * box.width_cm * box.height_cm) / 5;
  return std::max(volumetric_grams, actual_grams);
}

}  // namespace shipping
