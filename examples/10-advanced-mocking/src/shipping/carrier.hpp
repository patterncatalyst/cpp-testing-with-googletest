// The collaborator the unit under test depends on. In production a real
// implementation would talk to a carrier's network service; the test replaces it
// with a mock, so Dispatcher can be tested without any network.
#ifndef SHIPPING_CARRIER_HPP
#define SHIPPING_CARRIER_HPP

#include <string>
#include <string_view>

namespace shipping {

// A seam: an abstract interface Dispatcher talks to. Making it abstract is what
// lets a mock stand in for the real carrier under test.
class CarrierGateway {
 public:
  CarrierGateway() = default;
  CarrierGateway(const CarrierGateway&) = default;
  CarrierGateway& operator=(const CarrierGateway&) = default;
  CarrierGateway(CarrierGateway&&) = default;
  CarrierGateway& operator=(CarrierGateway&&) = default;
  virtual ~CarrierGateway() = default;

  // Whether the carrier serves a destination region at all.
  [[nodiscard]] virtual bool serves(std::string_view region) const = 0;

  // The price in cents to carry a parcel of this weight to the region.
  [[nodiscard]] virtual int quote_cents(int weight_grams,
                                        std::string_view region) const = 0;

  // Book the shipment and return a booking reference.
  virtual std::string book(int weight_grams, std::string_view region) = 0;
};

}  // namespace shipping

#endif  // SHIPPING_CARRIER_HPP
