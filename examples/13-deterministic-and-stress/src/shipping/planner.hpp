// Two units built to be tested deterministically. Each pushes a source of
// nondeterminism out through a seam the test controls: QuoteCache takes the
// current time as a parameter instead of reading a clock, and choose_backup_carrier
// takes the random engine instead of owning one. A test supplies a fixed time and a
// seeded engine, so the result is the same on every run.
#ifndef SHIPPING_PLANNER_HPP
#define SHIPPING_PLANNER_HPP

#include <chrono>
#include <optional>
#include <random>
#include <span>
#include <string>

namespace shipping {

// A quote that is valid for a fixed time-to-live. Freshness is decided against a
// caller-supplied "now", never against a clock read inside the class, so a test
// chooses the time rather than sleeping to let it pass.
class QuoteCache {
 public:
  using TimePoint = std::chrono::steady_clock::time_point;

  explicit QuoteCache(std::chrono::seconds ttl) : ttl_(ttl) {}

  void store(int cents, TimePoint at);

  // The cached cents if still within the TTL at `now`, otherwise nullopt.
  [[nodiscard]] std::optional<int> get(TimePoint now) const;

 private:
  std::chrono::seconds ttl_;
  int cents_ = 0;
  TimePoint stored_at_;
  bool has_value_ = false;
};

// Pick a backup carrier at random from the candidates. The engine is passed in,
// so a test seeds it and gets a deterministic choice; production passes a
// randomly-seeded engine. Precondition: candidates is non-empty.
[[nodiscard]] std::string choose_backup_carrier(
    std::span<const std::string> candidates, std::mt19937& engine);

}  // namespace shipping

#endif  // SHIPPING_PLANNER_HPP
