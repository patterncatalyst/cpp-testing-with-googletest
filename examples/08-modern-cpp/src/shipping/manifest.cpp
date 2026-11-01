#include "shipping/manifest.hpp"

#include <algorithm>
#include <functional>
#include <ranges>

namespace shipping {

int heavy_parcel_weight(const std::vector<Parcel>& parcels, int threshold_grams) {
  // Lazy pipeline: keep the parcels at or above the threshold, then project each
  // to its weight. Neither view touches an element until the fold pulls it.
  auto heavy = parcels | std::views::filter([threshold_grams](const Parcel& parcel) {
                 return parcel.weight_grams >= threshold_grams;
               }) |
               std::views::transform(&Parcel::weight_grams);

  // fold_left (C++23) walks the view once, summing from a seed of 0.
  return std::ranges::fold_left(heavy, 0, std::plus<>{});
}

}  // namespace shipping
