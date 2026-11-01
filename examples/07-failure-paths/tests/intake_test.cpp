#include "shipping/intake.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace {

// ── std::expected: a recoverable failure is a return value ────────────────────

TEST(ParseWeight, ReturnsGramsForValidText) {
  const std::expected<int, shipping::ParseError> result =
      shipping::parse_weight("2500");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 2500);
}

TEST(ParseWeight, RejectsEmptyTextWithEmptyError) {
  const auto result = shipping::parse_weight("");
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), shipping::ParseError::Empty);
}

TEST(ParseWeight, RejectsNonNumericText) {
  EXPECT_EQ(shipping::parse_weight("12kg").error(),
            shipping::ParseError::NotANumber);
}

TEST(ParseWeight, RejectsNonPositiveText) {
  EXPECT_EQ(shipping::parse_weight("0").error(),
            shipping::ParseError::NotPositive);
  EXPECT_EQ(shipping::parse_weight("-5").error(),
            shipping::ParseError::NotPositive);
}

// ── Exceptions: inspecting the message, not just the type ──────────────────────

TEST(ParseWeightOrThrow, ReturnsGramsForValidText) {
  EXPECT_EQ(shipping::parse_weight_or_throw("2500"), 2500);
}

TEST(ParseWeightOrThrow, ThrowsWithAMessageNamingTheReason) {
  try {
    (void)shipping::parse_weight_or_throw("");
    FAIL() << "expected parse_weight_or_throw to throw";
  } catch (const std::invalid_argument& error) {
    EXPECT_STREQ(error.what(), "weight text is empty");
  }
}

// ── Death tests: a broken invariant aborts the process ────────────────────────

TEST(RemainingCapacityDeathTest, AbortsWhenUsedExceedsTotal) {
  EXPECT_DEATH((void)shipping::remaining_capacity(5, 10),
               "used must not exceed total");
}

TEST(RemainingCapacity, ReturnsTheDifferenceWhenValid) {
  EXPECT_EQ(shipping::remaining_capacity(10, 4), 6);
}

}  // namespace
