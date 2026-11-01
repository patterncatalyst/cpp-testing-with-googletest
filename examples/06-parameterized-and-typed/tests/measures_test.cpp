#include "shipping/clamp.hpp"
#include "shipping/delivery_band.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

// ── Parameterized: one body, many (input, expected) rows ──────────────────────

struct BandCase {
  int weight_grams;
  shipping::Band expected;
};

class DeliveryBandTest : public ::testing::TestWithParam<BandCase> {};

TEST_P(DeliveryBandTest, ClassifiesByWeight) {
  const BandCase& row = GetParam();
  EXPECT_EQ(shipping::delivery_band(row.weight_grams), row.expected);
}

INSTANTIATE_TEST_SUITE_P(
    Boundaries, DeliveryBandTest,
    ::testing::Values(BandCase{1, shipping::Band::Letter},
                      BandCase{500, shipping::Band::Letter},
                      BandCase{501, shipping::Band::Parcel},
                      BandCase{30000, shipping::Band::Parcel},
                      BandCase{30001, shipping::Band::Freight}),
    [](const ::testing::TestParamInfo<BandCase>& info) {
      // Give each row a readable test name instead of an index.
      return "w" + std::to_string(info.param.weight_grams);
    });

// ── Typed: one body, many types ───────────────────────────────────────────────

template <typename T>
class ClampNonNegativeTest : public ::testing::Test {};

using ArithmeticTypes = ::testing::Types<int, long, double>;
TYPED_TEST_SUITE(ClampNonNegativeTest, ArithmeticTypes);

TYPED_TEST(ClampNonNegativeTest, LeavesNonNegativeValuesUnchanged) {
  EXPECT_EQ(shipping::clamp_non_negative(TypeParam{7}), TypeParam{7});
}

TYPED_TEST(ClampNonNegativeTest, RaisesNegativeValuesToZero) {
  EXPECT_EQ(shipping::clamp_non_negative(TypeParam{-3}), TypeParam{0});
}

}  // namespace
