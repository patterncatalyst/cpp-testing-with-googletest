#include "shipping/rate_quote.hpp"

#include <stdexcept>

namespace shipping {
namespace {

int base_cents(Zone zone) {
  switch (zone) {
    case Zone::Domestic:
      return 500;
    case Zone::Regional:
      return 900;
    case Zone::International:
      return 1800;
  }
  return 0;  // unreachable for a valid Zone
}

int per_kilogram_cents(Zone zone) {
  switch (zone) {
    case Zone::Domestic:
      return 120;
    case Zone::Regional:
      return 200;
    case Zone::International:
      return 450;
  }
  return 0;  // unreachable for a valid Zone
}

}  // namespace

int quote_cents(int weight_grams, Zone zone) {
  if (weight_grams <= 0) {
    throw std::invalid_argument("weight_grams must be positive");
  }
  const int kilograms = (weight_grams + 999) / 1000;  // round up to whole kg
  return base_cents(zone) + (kilograms * per_kilogram_cents(zone));
}

}  // namespace shipping
