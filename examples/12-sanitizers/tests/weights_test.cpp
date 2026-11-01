#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "shipping/weights.hpp"

namespace {

TEST(TotalWeight, SumsABatch) {
  const std::vector<int> weights{100, 250, 300, 50};
  EXPECT_EQ(shipping::total_weight(weights), 700);
}

// A batch whose 32-bit sum would overflow: 100 parcels of a billion grams is
// 1e11, far past INT_MAX. The 64-bit accumulator handles it, and this is the
// case the chapter uses to show UBSan catching the broken 32-bit version.
TEST(TotalWeight, HandlesALargeBatchWithoutOverflow) {
  const std::vector<int> weights(100, 1'000'000'000);
  EXPECT_EQ(shipping::total_weight(weights), 100'000'000'000LL);
}

TEST(MaxWeight, FindsTheLargestInTheBatch) {
  const std::vector<int> weights{100, 900, 250, 400};
  EXPECT_EQ(shipping::max_weight(weights), 900);
}

TEST(MaxWeight, HandlesASingleElement) {
  const std::array<int, 1> weights{42};
  EXPECT_EQ(shipping::max_weight(weights), 42);
}

// The parallel sum must match the sequential one for the same batch.
TEST(ParallelTotal, MatchesTheSequentialTotal) {
  const std::vector<int> weights{10, 20, 30, 40, 50, 60, 70};
  EXPECT_EQ(shipping::parallel_total(weights), shipping::total_weight(weights));
}

}  // namespace
