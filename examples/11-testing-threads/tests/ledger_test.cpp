#include <functional>
#include <future>
#include <latch>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "shipping/ledger.hpp"

namespace {

using shipping::ShipmentLedger;

// Launch n threads that each wait on a latch, release them together so their work
// overlaps as much as possible, then join. A simultaneous start gives a racy
// implementation the best chance to misbehave, which is what a concurrency test
// wants. The jthread destructors join when the vector goes out of scope.
void run_concurrently(int n_threads, const std::function<void(int)>& work) {
  std::latch start{1};
  std::vector<std::jthread> workers;
  workers.reserve(static_cast<std::size_t>(n_threads));
  for (int i = 0; i < n_threads; ++i) {
    workers.emplace_back([&work, &start, i] {
      start.wait();  // block until every worker is ready
      work(i);
    });
  }
  start.count_down();  // release all workers at once
}  // workers join here

// A sanity check with no threads: the ledger does basic arithmetic.
TEST(ShipmentLedger, RecordsCountAndTotalSequentially) {
  ShipmentLedger ledger;
  ledger.record(100);
  ledger.record(250);
  EXPECT_EQ(ledger.count(), 2);
  EXPECT_EQ(ledger.total_grams(), 350);
}

// The core concurrency test: many threads record at once, and the aggregate is
// deterministic even though the interleaving is not. A lost update from a race
// would leave the count or the total short.
TEST(ShipmentLedger, ConcurrentRecordsAreAllCountedAndSummed) {
  ShipmentLedger ledger;
  constexpr int thread_count = 8;
  constexpr int records_per_thread = 1000;
  constexpr int grams = 250;

  run_concurrently(thread_count, [&ledger](int /*thread_index*/) {
    for (int j = 0; j < records_per_thread; ++j) {
      ledger.record(grams);
    }
  });

  EXPECT_EQ(ledger.count(), thread_count * records_per_thread);
  EXPECT_EQ(ledger.total_grams(), thread_count * records_per_thread * grams);
}

// std::async returns a value through a future, and get() is the synchronization
// point: it waits for the task to finish before the test reads the ledger.
TEST(ShipmentLedger, AsyncWorkersReturnCountsThatMatchTheLedger) {
  ShipmentLedger ledger;
  const auto worker = [&ledger](int parcels, int grams) {
    for (int i = 0; i < parcels; ++i) {
      ledger.record(grams);
    }
    return parcels;
  };

  std::future<int> first = std::async(std::launch::async, worker, 300, 100);
  std::future<int> second = std::async(std::launch::async, worker, 700, 100);
  const int recorded = first.get() + second.get();  // get() joins both tasks

  EXPECT_EQ(recorded, 1000);
  EXPECT_EQ(ledger.count(), 1000);
  EXPECT_EQ(ledger.total_grams(), 100000);
}

}  // namespace
