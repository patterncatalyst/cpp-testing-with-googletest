#include "shipping/ledger.hpp"

namespace shipping {

void ShipmentLedger::record(int grams) {
  const std::scoped_lock lock(mutex_);
  count_ += 1;
  total_grams_ += grams;
}

int ShipmentLedger::count() const {
  const std::scoped_lock lock(mutex_);
  return count_;
}

int ShipmentLedger::total_grams() const {
  const std::scoped_lock lock(mutex_);
  return total_grams_;
}

}  // namespace shipping
