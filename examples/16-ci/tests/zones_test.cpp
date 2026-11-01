#include <gtest/gtest.h>

#include "shipping/zones.hpp"

namespace {

TEST(DeliveryZone, DomesticIsZoneOne) {
  EXPECT_EQ(shipping::delivery_zone("domestic"), 1);
}

TEST(DeliveryZone, EuAndUkAreZoneTwo) {
  EXPECT_EQ(shipping::delivery_zone("EU"), 2);
  EXPECT_EQ(shipping::delivery_zone("UK"), 2);
}

TEST(DeliveryZone, EverywhereElseIsZoneThree) {
  EXPECT_EQ(shipping::delivery_zone("APAC"), 3);
}

}  // namespace
