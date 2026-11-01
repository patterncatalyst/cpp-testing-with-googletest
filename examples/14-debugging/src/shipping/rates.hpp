// A tiered shipping rate and a batch total. The functions are small enough to
// step through under a debugger, which is the subject of this chapter: the chapter
// plants a boundary bug in tiered_cost_cents and walks through finding it in GDB.
#ifndef SHIPPING_RATES_HPP
#define SHIPPING_RATES_HPP

#include <span>

namespace shipping {

// The cost in cents to ship one parcel, by weight tier:
//   up to   500 g -> 500
//   up to 2000 g -> 1200
//   up to 5000 g -> 2500
//   heavier       -> 4000
// A non-positive weight is treated as the lightest tier.
[[nodiscard]] int tiered_cost_cents(int weight_grams);

// The total cost of a batch, summing the tiered cost of each parcel.
[[nodiscard]] int batch_cost_cents(std::span<const int> weights);

}  // namespace shipping

#endif  // SHIPPING_RATES_HPP
