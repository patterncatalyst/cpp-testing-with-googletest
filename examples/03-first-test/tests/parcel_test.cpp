#include "shipping/parcel.hpp"

#include <gtest/gtest.h>

namespace {

TEST(BillableWeight, BillsVolumetricForALightBulkyParcel) {
  // 40 x 30 x 25 cm = 30000 cm3 -> 6000 g volumetric, above the 2000 g scale.
  EXPECT_EQ(shipping::billable_weight_grams(
                shipping::Dimensions{40, 30, 25}, 2000),
            6000);
}

TEST(BillableWeight, BillsActualForASmallHeavyParcel) {
  // 10 x 10 x 10 cm = 1000 cm3 -> 200 g volumetric, below the 5000 g scale.
  EXPECT_EQ(shipping::billable_weight_grams(
                shipping::Dimensions{10, 10, 10}, 5000),
            5000);
}

TEST(BillableWeight, BillsEitherWhenTheyAreEqual) {
  // 20 x 10 x 10 cm = 2000 cm3 -> 400 g volumetric, equal to the scale weight.
  EXPECT_EQ(shipping::billable_weight_grams(
                shipping::Dimensions{20, 10, 10}, 400),
            400);
}

}  // namespace
