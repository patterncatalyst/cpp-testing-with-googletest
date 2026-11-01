// A small delivery-zone lookup. The code is deliberately modest; this chapter is
// about the pipeline that runs the tests, not the logic under test.
#ifndef SHIPPING_ZONES_HPP
#define SHIPPING_ZONES_HPP

#include <string_view>

namespace shipping {

// The delivery zone for a destination region:
//   domestic           -> 1
//   EU or UK           -> 2
//   anywhere else      -> 3
[[nodiscard]] int delivery_zone(std::string_view region);

}  // namespace shipping

#endif  // SHIPPING_ZONES_HPP
