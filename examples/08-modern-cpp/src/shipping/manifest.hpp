// A range pipeline over a manifest of parcels.
#ifndef SHIPPING_MANIFEST_HPP
#define SHIPPING_MANIFEST_HPP

#include <vector>

#include "shipping/parcel.hpp"

namespace shipping {

// Total grams across the parcels at or above a weight threshold. Implemented as
// a lazy views pipeline (filter then project) folded to a single sum.
int heavy_parcel_weight(const std::vector<Parcel>& parcels, int threshold_grams);

}  // namespace shipping

#endif  // SHIPPING_MANIFEST_HPP
