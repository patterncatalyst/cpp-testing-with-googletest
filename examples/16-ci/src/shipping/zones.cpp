#include "shipping/zones.hpp"

namespace shipping {

int delivery_zone(std::string_view region) {
  if (region == "domestic") {
    return 1;
  }
  if (region == "EU" || region == "UK") {
    return 2;
  }
  return 3;
}

}  // namespace shipping
