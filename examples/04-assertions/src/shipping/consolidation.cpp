#include "shipping/consolidation.hpp"

#include <stdexcept>

namespace shipping {

std::vector<int> split_into_shipments(int total_grams, int max_grams) {
  if (max_grams <= 0) {
    throw std::invalid_argument("max_grams must be positive");
  }
  if (total_grams < 0) {
    throw std::invalid_argument("total_grams must be non-negative");
  }

  std::vector<int> shipments;
  int remaining = total_grams;
  while (remaining > max_grams) {
    shipments.push_back(max_grams);
    remaining -= max_grams;
  }
  if (remaining > 0) {
    shipments.push_back(remaining);
  }
  return shipments;
}

}  // namespace shipping
