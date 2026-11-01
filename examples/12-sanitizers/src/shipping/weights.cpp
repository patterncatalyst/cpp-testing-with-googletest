#include "shipping/weights.hpp"

#include <algorithm>
#include <future>

namespace shipping {

long long total_weight(std::span<const int> weights) {
  long long sum = 0;  // 64-bit: a large batch cannot overflow this
  for (const int weight : weights) {
    sum += weight;
  }
  return sum;
}

int max_weight(std::span<const int> weights) {
  int largest = weights[0];
  for (std::size_t i = 1; i < weights.size(); ++i) {  // in bounds: [1, size)
    largest = std::max(weights[i], largest);
  }
  return largest;
}

long long parallel_total(std::span<const int> weights) {
  const std::size_t middle = weights.size() / 2;
  // Each task sums its own half into its own return value: no shared state.
  auto sum_half = [](std::span<const int> part) {
    long long sum = 0;
    for (const int weight : part) {
      sum += weight;
    }
    return sum;
  };
  std::future<long long> second =
      std::async(std::launch::async, sum_half, weights.subspan(middle));
  const long long first = sum_half(weights.first(middle));
  return first + second.get();
}

}  // namespace shipping
