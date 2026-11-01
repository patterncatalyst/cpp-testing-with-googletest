// Coroutine generators over a shipment.
#ifndef SHIPPING_BATCHING_HPP
#define SHIPPING_BATCHING_HPP

#include <generator>

namespace shipping {

// Yield the weight of each successive batch when a total is split into batches
// of at most batch_grams. The last batch holds the remainder. A finite sequence.
std::generator<int> batch_weights(int total_grams, int batch_grams);

// Yield a running total that grows by step each time, without end. The sequence
// is infinite; because a generator is lazy, a caller takes only what it needs.
std::generator<int> running_total(int step);

}  // namespace shipping

#endif  // SHIPPING_BATCHING_HPP
