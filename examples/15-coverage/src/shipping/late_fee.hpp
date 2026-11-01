// A late-fee calculation with several branches, used to show source-based coverage:
// which lines, regions, and branches the test suite actually exercises. The branch
// structure is the point, so coverage has something to measure.
#ifndef SHIPPING_LATE_FEE_HPP
#define SHIPPING_LATE_FEE_HPP

namespace shipping {

// The late fee in cents for a parcel returned this many days late:
//   not late (days_late <= 0)        -> 0
//   a member within a 3-day grace    -> 0
//   otherwise                        -> 100 per day, capped at 2000
[[nodiscard]] int late_fee_cents(int days_late, bool is_member);

}  // namespace shipping

#endif  // SHIPPING_LATE_FEE_HPP
