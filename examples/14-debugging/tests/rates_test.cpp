#include <vector>

#include <gtest/gtest.h>

#include "shipping/rates.hpp"

namespace {

// One case per tier, each testing the boundary: the heaviest weight that still
// falls in a tier, and the lightest that spills into the next. Boundary cases are
// where a tiered function goes wrong, and where a debugger earns its time.
TEST(TieredCost, ChargesByWeightTier) {
  EXPECT_EQ(shipping::tiered_cost_cents(500), 500);    // top of tier 1
  EXPECT_EQ(shipping::tiered_cost_cents(501), 1200);   // bottom of tier 2
  EXPECT_EQ(shipping::tiered_cost_cents(2000), 1200);  // top of tier 2
  EXPECT_EQ(shipping::tiered_cost_cents(2001), 2500);  // bottom of tier 3
  EXPECT_EQ(shipping::tiered_cost_cents(5000), 2500);  // top of tier 3
  EXPECT_EQ(shipping::tiered_cost_cents(5001), 4000);  // heaviest tier
}

TEST(TieredCost, TreatsNonPositiveAsTheLightestTier) {
  EXPECT_EQ(shipping::tiered_cost_cents(0), 500);
  EXPECT_EQ(shipping::tiered_cost_cents(-10), 500);
}

TEST(BatchCost, SumsTheTieredCostOfEachParcel) {
  const std::vector<int> weights{300, 1500, 6000};  // 500 + 1200 + 4000
  EXPECT_EQ(shipping::batch_cost_cents(weights), 5700);
}

TEST(BatchCost, IsZeroForAnEmptyBatch) {
  const std::vector<int> weights;
  EXPECT_EQ(shipping::batch_cost_cents(weights), 0);
}

}  // namespace
