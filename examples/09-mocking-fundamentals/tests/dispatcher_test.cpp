#include <optional>
#include <string>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "shipping/carrier.hpp"
#include "shipping/dispatcher.hpp"

namespace {

using ::testing::_;
using ::testing::Ge;
using ::testing::Return;
using ::testing::StrEq;

// The mock: one MOCK_METHOD per virtual on the interface. The trailing specs
// mirror the signature's qualifiers: a const method is (const, override), a
// mutating one is (override). GoogleMock generates the body.
class MockCarrierGateway : public shipping::CarrierGateway {
 public:
  MOCK_METHOD(bool, serves, (std::string_view region), (const, override));
  MOCK_METHOD(int, quote_cents, (int weight_grams, std::string_view region),
              (const, override));
  MOCK_METHOD(std::string, book, (int weight_grams, std::string_view region),
              (override));
};

// Happy path: served and within budget. The three expectations describe the
// whole interaction, and book's return value flows back out of dispatch.
TEST(Dispatcher, BooksWhenServedAndWithinBudget) {
  MockCarrierGateway carrier;
  EXPECT_CALL(carrier, serves(StrEq("EU"))).WillOnce(Return(true));
  EXPECT_CALL(carrier, quote_cents(2500, StrEq("EU"))).WillOnce(Return(1500));
  EXPECT_CALL(carrier, book(2500, StrEq("EU"))).WillOnce(Return("BK-100"));

  shipping::Dispatcher dispatcher(carrier);
  const std::optional<std::string> reference =
      dispatcher.dispatch(2500, "EU", 2000);

  ASSERT_TRUE(reference.has_value());
  EXPECT_EQ(*reference, "BK-100");
}

// An unserved region short-circuits: quote_cents and book must never be called.
// Times(0) turns an unwanted call into a failure.
TEST(Dispatcher, DoesNotQuoteOrBookWhenRegionIsUnserved) {
  MockCarrierGateway carrier;
  EXPECT_CALL(carrier, serves(StrEq("XX"))).WillOnce(Return(false));
  EXPECT_CALL(carrier, quote_cents(_, _)).Times(0);
  EXPECT_CALL(carrier, book(_, _)).Times(0);

  shipping::Dispatcher dispatcher(carrier);
  EXPECT_FALSE(dispatcher.dispatch(2500, "XX", 10000).has_value());
}

// Over budget: the quote is fetched but nothing is booked.
TEST(Dispatcher, DoesNotBookWhenQuoteExceedsBudget) {
  MockCarrierGateway carrier;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(true));
  EXPECT_CALL(carrier, quote_cents(_, _)).WillOnce(Return(5000));
  EXPECT_CALL(carrier, book(_, _)).Times(0);

  shipping::Dispatcher dispatcher(carrier);
  EXPECT_FALSE(dispatcher.dispatch(2500, "EU", 2000).has_value());
}

// Matchers describe arguments without pinning exact values. Ge(0) accepts any
// non-negative weight; _ accepts any region. The test still asserts the result.
TEST(Dispatcher, PassesTheWeightThroughToTheQuote) {
  MockCarrierGateway carrier;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(true));
  EXPECT_CALL(carrier, quote_cents(Ge(0), _)).WillOnce(Return(800));
  EXPECT_CALL(carrier, book(Ge(0), _)).WillOnce(Return("BK-200"));

  shipping::Dispatcher dispatcher(carrier);
  EXPECT_EQ(dispatcher.dispatch(300, "US", 1000), "BK-200");
}

// ON_CALL sets default behavior without asserting the call happens; the single
// EXPECT_CALL then focuses the test on the one interaction that matters, here
// that book is reached with the right reference. The serves and quote_cents
// stubs are background, not the subject of the test.
TEST(Dispatcher, OnCallStubsBackgroundInteractions) {
  MockCarrierGateway carrier;
  ON_CALL(carrier, serves(_)).WillByDefault(Return(true));
  ON_CALL(carrier, quote_cents(_, _)).WillByDefault(Return(500));
  EXPECT_CALL(carrier, book(2500, StrEq("EU"))).WillOnce(Return("BK-300"));

  shipping::Dispatcher dispatcher(carrier);
  EXPECT_EQ(dispatcher.dispatch(2500, "EU", 2000), "BK-300");
}

}  // namespace
