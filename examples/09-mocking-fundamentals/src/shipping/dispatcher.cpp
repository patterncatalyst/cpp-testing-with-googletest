#include "shipping/dispatcher.hpp"

namespace shipping {

std::optional<std::string> Dispatcher::dispatch(int weight_grams,
                                                std::string_view region,
                                                int budget_cents) {
  if (!carrier_.serves(region)) {
    return std::nullopt;  // unserved: never quote, never book
  }
  const int cost = carrier_.quote_cents(weight_grams, region);
  if (cost > budget_cents) {
    return std::nullopt;  // over budget: never book
  }
  return carrier_.book(weight_grams, region);
}

}  // namespace shipping
