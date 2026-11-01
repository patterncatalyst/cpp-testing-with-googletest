#include <ranges>
#include <vector>

#include <gtest/gtest.h>

#include "shipping/batching.hpp"
#include "shipping/manifest.hpp"
#include "shipping/parcel.hpp"

namespace {

// ── Concepts ─────────────────────────────────────────────────────────────────
// The concept is machinery the compiler checks, so the first tests run at
// compile time. A static_assert that fails stops the build with a clear message;
// if the file compiles, these have already passed.
static_assert(shipping::Weighable<shipping::Parcel>,
              "Parcel exposes weight_grams, so it must be Weighable");
static_assert(!shipping::Weighable<int>,
              "a plain int has no weight_grams, so it must not be Weighable");

// The concept is structural: any type with the right shape satisfies it, not
// only Parcel. A crate that also carries a volume still qualifies.
struct Crate {
  int weight_grams;
  double volume_litres;
};
static_assert(shipping::Weighable<Crate>, "Crate has weight_grams too");

// A concept is also a constexpr bool, so the same checks read naturally as
// ordinary runtime expectations when you want them in the test report.
TEST(WeighableConcept, ParcelSatisfiesItAndIntDoesNot) {
  EXPECT_TRUE(shipping::Weighable<shipping::Parcel>);
  EXPECT_FALSE(shipping::Weighable<int>);
}

TEST(HeavierWeight, ReturnsTheLargerOfTwoWeights) {
  const shipping::Parcel light{.weight_grams = 400, .fragile = false};
  const shipping::Parcel heavy{.weight_grams = 1500, .fragile = true};
  EXPECT_EQ(shipping::heavier_weight(light, heavy), 1500);
  EXPECT_EQ(shipping::heavier_weight(heavy, light), 1500);
}

TEST(HeavierWeight, WorksForAnyWeighableType) {
  const Crate small{.weight_grams = 2000, .volume_litres = 30.0};
  const Crate large{.weight_grams = 8000, .volume_litres = 90.0};
  EXPECT_EQ(shipping::heavier_weight(small, large), 8000);
}

// ── Ranges ───────────────────────────────────────────────────────────────────
TEST(HeavyParcelWeight, SumsOnlyParcelsAtOrAboveTheThreshold) {
  const std::vector<shipping::Parcel> manifest{
      {.weight_grams = 200, .fragile = false},
      {.weight_grams = 1200, .fragile = false},
      {.weight_grams = 3000, .fragile = true},
  };
  EXPECT_EQ(shipping::heavy_parcel_weight(manifest, 1000), 4200);
}

TEST(HeavyParcelWeight, ThresholdIsInclusive) {
  const std::vector<shipping::Parcel> manifest{
      {.weight_grams = 1000, .fragile = false},
      {.weight_grams = 999, .fragile = false},
  };
  // Only the 1000 g parcel qualifies; 999 g is below the threshold.
  EXPECT_EQ(shipping::heavy_parcel_weight(manifest, 1000), 1000);
}

TEST(HeavyParcelWeight, ReturnsZeroWhenNothingQualifies) {
  const std::vector<shipping::Parcel> manifest{
      {.weight_grams = 100, .fragile = false},
      {.weight_grams = 200, .fragile = false},
  };
  // The filter empties the range, so the fold returns its seed.
  EXPECT_EQ(shipping::heavy_parcel_weight(manifest, 1000), 0);
}

// ── Coroutines ───────────────────────────────────────────────────────────────
TEST(BatchWeights, SplitsIntoFullBatchesPlusARemainder) {
  std::vector<int> batches;
  for (const int weight : shipping::batch_weights(2500, 1000)) {
    batches.push_back(weight);
  }
  EXPECT_EQ(batches, (std::vector<int>{1000, 1000, 500}));
}

TEST(BatchWeights, DividesEvenlyWithNoRemainder) {
  const std::vector<int> batches =
      shipping::batch_weights(2000, 1000) | std::ranges::to<std::vector>();
  EXPECT_EQ(batches, (std::vector<int>{1000, 1000}));
}

TEST(BatchWeights, YieldsNothingForAZeroTotal) {
  const std::vector<int> batches =
      shipping::batch_weights(0, 1000) | std::ranges::to<std::vector>();
  EXPECT_TRUE(batches.empty());
}

TEST(RunningTotal, IsLazySoAnInfiniteSequenceIsSafeToTake) {
  std::vector<int> first_three;
  // running_total never returns on its own; views::take pulls exactly three
  // values and then stops asking, which is the only reason this terminates.
  for (const int value : shipping::running_total(500) | std::views::take(3)) {
    first_three.push_back(value);
  }
  EXPECT_EQ(first_three, (std::vector<int>{500, 1000, 1500}));
}

}  // namespace
