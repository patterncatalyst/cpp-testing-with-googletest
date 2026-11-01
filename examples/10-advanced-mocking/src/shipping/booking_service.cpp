#include "shipping/booking_service.hpp"

#include <string>

namespace shipping {

std::optional<std::string> BookingService::book(int weight_grams,
                                                std::string_view region,
                                                int budget_cents) {
  if (!carrier_.serves(region)) {
    audit_.record("rejected: unserved");
    return std::nullopt;
  }
  const int cost = carrier_.quote_cents(weight_grams, region);
  if (cost > budget_cents) {
    audit_.record("rejected: over budget");
    return std::nullopt;
  }
  std::string reference = carrier_.book(weight_grams, region);
  audit_.record("booked " + reference);  // after the booking, never before
  return reference;
}

}  // namespace shipping
