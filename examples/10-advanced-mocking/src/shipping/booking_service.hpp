// The unit under test. It depends on two collaborators injected by reference: a
// CarrierGateway to price and book, and an AuditLog to record the outcome. Both
// are abstract interfaces, so a test can substitute a mock or a fake for each.
#ifndef SHIPPING_BOOKING_SERVICE_HPP
#define SHIPPING_BOOKING_SERVICE_HPP

#include <optional>
#include <string>
#include <string_view>

#include "shipping/audit.hpp"
#include "shipping/carrier.hpp"

namespace shipping {

class BookingService {
 public:
  BookingService(CarrierGateway& carrier, AuditLog& audit)
      : carrier_(carrier), audit_(audit) {}

  // Book a parcel to a region within a budget. On success, book the carrier and
  // then record the booking in the audit log, in that order. On a rejection,
  // record the reason and book nothing. Returns the booking reference or nullopt.
  std::optional<std::string> book(int weight_grams, std::string_view region,
                                  int budget_cents);

 private:
  CarrierGateway& carrier_;
  AuditLog& audit_;
};

}  // namespace shipping

#endif  // SHIPPING_BOOKING_SERVICE_HPP
