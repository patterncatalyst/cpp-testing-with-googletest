#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "shipping/audit.hpp"
#include "shipping/booking_service.hpp"
#include "shipping/carrier.hpp"

namespace {

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::StrictMock;

class MockCarrierGateway : public shipping::CarrierGateway {
 public:
  MOCK_METHOD(bool, serves, (std::string_view region), (const, override));
  MOCK_METHOD(int, quote_cents, (int weight_grams, std::string_view region),
              (const, override));
  MOCK_METHOD(std::string, book, (int weight_grams, std::string_view region),
              (override));
};

class MockAuditLog : public shipping::AuditLog {
 public:
  MOCK_METHOD(void, record, (const std::string& event), (override));
};

// A fake: a real, lightweight implementation that records state a test inspects
// afterward, rather than expectations checked during the call.
class FakeAuditLog : public shipping::AuditLog {
 public:
  void record(const std::string& event) override { events.push_back(event); }
  std::vector<std::string> events;
};

// InSequence makes every EXPECT_CALL in its scope ordered: the booking must come
// before the audit record, and the three carrier steps in their own order.
TEST(BookingService, BooksThenRecordsInOrder) {
  MockCarrierGateway carrier;
  MockAuditLog audit;
  {
    const InSequence seq;
    EXPECT_CALL(carrier, serves(_)).WillOnce(Return(true));
    EXPECT_CALL(carrier, quote_cents(_, _)).WillOnce(Return(1000));
    EXPECT_CALL(carrier, book(_, _)).WillOnce(Return("BK-1"));
    EXPECT_CALL(audit, record("booked BK-1"));
  }

  shipping::BookingService service(carrier, audit);
  EXPECT_EQ(service.book(2500, "EU", 2000), "BK-1");
}

// NiceMock silences the uninteresting-call warning for the audit log, which this
// test is not about. Only the carrier interaction is asserted.
TEST(BookingService, NiceMockSilencesTheBackgroundAudit) {
  MockCarrierGateway carrier;
  NiceMock<MockAuditLog> audit;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(false));

  shipping::BookingService service(carrier, audit);
  EXPECT_FALSE(service.book(2500, "XX", 9999).has_value());
}

// StrictMock turns any unexpected call into a failure, so expecting only serves
// asserts that quote_cents and book never run, without writing Times(0) for each.
TEST(BookingService, StrictMockPinsExactlyTheExpectedCalls) {
  StrictMock<MockCarrierGateway> carrier;
  NiceMock<MockAuditLog> audit;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(false));

  shipping::BookingService service(carrier, audit);
  EXPECT_FALSE(service.book(2500, "XX", 9999).has_value());
}

// SaveArg captures a call argument into a variable the test inspects afterward,
// so the exact audited message can be asserted.
TEST(BookingService, CapturesTheAuditedMessageWithSaveArg) {
  MockCarrierGateway carrier;
  MockAuditLog audit;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(true));
  EXPECT_CALL(carrier, quote_cents(_, _)).WillOnce(Return(1000));
  EXPECT_CALL(carrier, book(_, _)).WillOnce(Return("BK-9"));

  std::string recorded;
  EXPECT_CALL(audit, record(_)).WillOnce(SaveArg<0>(&recorded));

  shipping::BookingService service(carrier, audit);
  service.book(2500, "EU", 2000);
  EXPECT_EQ(recorded, "booked BK-9");
}

// A fake instead of a mock: drive two bookings, then inspect what the fake
// accumulated. The assertion is after the fact, on state, not on expectations.
TEST(BookingService, FakeAuditLogAccumulatesEverySuccess) {
  NiceMock<MockCarrierGateway> carrier;
  ON_CALL(carrier, serves(_)).WillByDefault(Return(true));
  ON_CALL(carrier, quote_cents(_, _)).WillByDefault(Return(500));
  ON_CALL(carrier, book(_, _)).WillByDefault(Return("BK-5"));
  FakeAuditLog audit;

  shipping::BookingService service(carrier, audit);
  service.book(2500, "EU", 2000);
  service.book(300, "US", 1000);

  ASSERT_EQ(audit.events.size(), 2U);
  EXPECT_EQ(audit.events[0], "booked BK-5");
  EXPECT_EQ(audit.events[1], "booked BK-5");
}

}  // namespace
