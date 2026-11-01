#ifndef SHIPPING_CONSOLIDATION_HPP
#define SHIPPING_CONSOLIDATION_HPP

#include <vector>

namespace shipping {

// Split a total weight into shipments no heavier than max_grams each. Every
// shipment but the last is filled to max_grams; the last carries the remainder.
// A zero total yields no shipments. `max_grams` must be positive and
// `total_grams` must be non-negative, or the call throws std::invalid_argument.
[[nodiscard]] std::vector<int> split_into_shipments(int total_grams,
                                                    int max_grams);

}  // namespace shipping

#endif  // SHIPPING_CONSOLIDATION_HPP
