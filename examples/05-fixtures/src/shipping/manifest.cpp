#include "shipping/manifest.hpp"

#include <numeric>
#include <stdexcept>

namespace shipping {

void Manifest::add(int weight_grams) {
  if (weight_grams <= 0) {
    throw std::invalid_argument("weight_grams must be positive");
  }
  weights_.push_back(weight_grams);
}

int Manifest::count() const { return static_cast<int>(weights_.size()); }

int Manifest::total_grams() const {
  return std::accumulate(weights_.begin(), weights_.end(), 0);
}

bool Manifest::is_empty() const { return weights_.empty(); }

}  // namespace shipping
