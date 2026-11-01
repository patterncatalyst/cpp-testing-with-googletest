#include "shipping/rate_quote.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace {

TEST(RateQuote, ChargesBasePlusPerKilogram) {
  // 500 base + 3 kg (2300 g rounds up) * 120 per kg = 860 cents.
  EXPECT_EQ(shipping::quote_cents(2300, shipping::Zone::Domestic), 860);
}

TEST(RateQuote, RoundsAPartialKilogramUp) {
  // 1 g still bills as one whole kilogram: 500 + 120 = 620.
  EXPECT_EQ(shipping::quote_cents(1, shipping::Zone::Domestic), 620);
}

TEST(RateQuote, ChargesMoreForFartherZones) {
  EXPECT_GT(shipping::quote_cents(1000, shipping::Zone::International),
            shipping::quote_cents(1000, shipping::Zone::Domestic));
}

TEST(RateQuote, RejectsNonPositiveWeight) {
  EXPECT_THROW((void)shipping::quote_cents(0, shipping::Zone::Domestic),
               std::invalid_argument);
}

}  // namespace
