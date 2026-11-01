// The unit under test. It holds a reference to a CarrierGateway and orchestrates
// a dispatch: check the region is served, get a quote, and book only if the quote
// fits the budget. Each step is a call to the collaborator, which is exactly what
// a mock lets a test observe and control.
#ifndef SHIPPING_DISPATCHER_HPP
#define SHIPPING_DISPATCHER_HPP

#include <optional>
#include <string>
#include <string_view>

#include "shipping/carrier.hpp"

namespace shipping {

class Dispatcher {
 public:
  explicit Dispatcher(CarrierGateway& carrier) : carrier_(carrier) {}

  // Book a parcel to a region within a budget. Returns the booking reference on
  // success, or nullopt if the carrier does not serve the region or the quote
  // exceeds the budget. In those two cases nothing is booked.
  std::optional<std::string> dispatch(int weight_grams, std::string_view region,
                                      int budget_cents);

 private:
  CarrierGateway& carrier_;
};

}  // namespace shipping

#endif  // SHIPPING_DISPATCHER_HPP
