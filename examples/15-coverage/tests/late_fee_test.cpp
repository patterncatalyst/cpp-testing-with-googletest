#include <gtest/gtest.h>

#include "shipping/late_fee.hpp"

namespace {

// These first three cases cover the early returns and the per-day fee, but not the
// cap. The chapter runs coverage over just this suite to show the cap branch red,
// then adds ChargesUpToACap below to close the gap.
TEST(LateFee, IsZeroWhenNotLate) {
  EXPECT_EQ(shipping::late_fee_cents(0, false), 0);
  EXPECT_EQ(shipping::late_fee_cents(-2, false), 0);
}

TEST(LateFee, IsZeroForAMemberWithinTheGrace) {
  EXPECT_EQ(shipping::late_fee_cents(3, true), 0);
}

TEST(LateFee, ChargesPerDayPastTheGrace) {
  EXPECT_EQ(shipping::late_fee_cents(5, false), 500);   // non-member, 5 days
  EXPECT_EQ(shipping::late_fee_cents(4, true), 400);    // member, past grace
}

// The case that exercises the cap branch: a long delay is clamped to 2000.
TEST(LateFee, ChargesUpToACap) {
  EXPECT_EQ(shipping::late_fee_cents(50, false), 2000);
}

}  // namespace
