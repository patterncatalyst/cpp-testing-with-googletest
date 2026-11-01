#include "shipping/planner.hpp"

namespace shipping {

void QuoteCache::store(int cents, TimePoint at) {
  cents_ = cents;
  stored_at_ = at;
  has_value_ = true;
}

std::optional<int> QuoteCache::get(TimePoint now) const {
  if (!has_value_ || now - stored_at_ >= ttl_) {
    return std::nullopt;
  }
  return cents_;
}

std::string choose_backup_carrier(std::span<const std::string> candidates,
                                  std::mt19937& engine) {
  std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
  return candidates[pick(engine)];
}

}  // namespace shipping
