// Three small, correct functions over a batch of parcel weights. Each is written
// to stay within the rules the sanitizers enforce; the chapter breaks each one in
// turn to show the matching sanitizer catching it: UBSan on overflow, ASan on an
// out-of-bounds read, TSan on a data race.
#ifndef SHIPPING_WEIGHTS_HPP
#define SHIPPING_WEIGHTS_HPP

#include <span>

namespace shipping {

// Sum of all weights. Accumulates into a 64-bit type so a large batch cannot
// overflow the way a 32-bit int would.
[[nodiscard]] long long total_weight(std::span<const int> weights);

// The largest weight in a non-empty batch.
[[nodiscard]] int max_weight(std::span<const int> weights);

// The same sum as total_weight, computed on two threads over halves of the batch
// and combined. Each thread owns its own partial, so there is no shared state to
// race on.
[[nodiscard]] long long parallel_total(std::span<const int> weights);

}  // namespace shipping

#endif  // SHIPPING_WEIGHTS_HPP
