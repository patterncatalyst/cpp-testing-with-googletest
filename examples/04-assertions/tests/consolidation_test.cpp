#include "shipping/consolidation.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SplitIntoShipments, FillsEachShipmentThenLeavesTheRemainder) {
  const auto shipments = shipping::split_into_shipments(2500, 1000);
  // Fatal: every line below indexes into the vector, so a wrong size must stop
  // the test here rather than read out of bounds.
  ASSERT_EQ(shipments.size(), 3U);
  EXPECT_EQ(shipments[0], 1000);
  EXPECT_EQ(shipments[1], 1000);
  EXPECT_EQ(shipments[2], 500);
}

TEST(SplitIntoShipments, LeavesNoTrailingShipmentOnAnExactMultiple) {
  const auto shipments = shipping::split_into_shipments(2000, 1000);
  ASSERT_EQ(shipments.size(), 2U);
  EXPECT_EQ(shipments[0], 1000);
  EXPECT_EQ(shipments[1], 1000);
}

TEST(SplitIntoShipments, AWholeVectorCanBeComparedInOneAssertion) {
  // std::vector has operator==, so size and contents are checked together.
  const std::vector<int> expected{1000, 1000, 500};
  EXPECT_EQ(shipping::split_into_shipments(2500, 1000), expected);
}

TEST(SplitIntoShipments, ReturnsNoShipmentsForZeroWeight) {
  EXPECT_TRUE(shipping::split_into_shipments(0, 1000).empty());
}

TEST(SplitIntoShipments, RejectsANonPositiveMax) {
  EXPECT_THROW((void)shipping::split_into_shipments(1000, 0),
               std::invalid_argument);
}

}  // namespace
