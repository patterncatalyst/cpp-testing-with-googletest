#include "shipping/rates.hpp"

namespace shipping {

int tiered_cost_cents(int weight_grams) {
  if (weight_grams <= 500) {
    return 500;
  }
  if (weight_grams <= 2000) {
    return 1200;
  }
  if (weight_grams <= 5000) {
    return 2500;
  }
  return 4000;
}

int batch_cost_cents(std::span<const int> weights) {
  int total = 0;
  for (const int weight : weights) {
    total += tiered_cost_cents(weight);
  }
  return total;
}

}  // namespace shipping
