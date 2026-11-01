#include "shipping/batching.hpp"

#include <algorithm>

namespace shipping {

std::generator<int> batch_weights(int total_grams, int batch_grams) {
  int remaining = total_grams;
  while (remaining > 0) {
    const int this_batch = std::min(remaining, batch_grams);
    co_yield this_batch;  // hand one value back, then suspend here
    remaining -= this_batch;
  }
}

std::generator<int> running_total(int step) {
  int total = 0;
  while (true) {  // no termination: the caller decides when to stop pulling
    total += step;
    co_yield total;
  }
}

}  // namespace shipping
