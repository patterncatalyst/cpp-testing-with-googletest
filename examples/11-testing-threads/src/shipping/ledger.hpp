// A thread-safe ledger: many worker threads record parcels into it at once, and
// a mutex keeps the running totals consistent. This is the unit under test in a
// chapter about testing threaded code; the tests drive it from several threads
// and assert the aggregate the concurrency must preserve.
#ifndef SHIPPING_LEDGER_HPP
#define SHIPPING_LEDGER_HPP

#include <mutex>

namespace shipping {

class ShipmentLedger {
 public:
  // Record one parcel of the given weight. Safe to call from many threads.
  void record(int grams);

  // The number of parcels recorded, and their total weight. Both take the lock,
  // so a reader never observes a half-updated pair.
  [[nodiscard]] int count() const;
  [[nodiscard]] int total_grams() const;

 private:
  mutable std::mutex mutex_;
  int count_ = 0;
  int total_grams_ = 0;
};

}  // namespace shipping

#endif  // SHIPPING_LEDGER_HPP
