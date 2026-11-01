#include "shipping/late_fee.hpp"

namespace shipping {

int late_fee_cents(int days_late, bool is_member) {
  if (days_late <= 0) {
    return 0;  // not late
  }
  if (is_member && days_late <= 3) {
    return 0;  // member grace period
  }
  const int fee = days_late * 100;
  if (fee > 2000) {
    return 2000;  // cap
  }
  return fee;
}

}  // namespace shipping
