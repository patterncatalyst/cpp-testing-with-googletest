#include <algorithm>
#include <chrono>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "shipping/planner.hpp"

namespace {

using namespace std::chrono_literals;
using shipping::QuoteCache;

// Time is a parameter, so the test picks the moment instead of sleeping. A quote
// is fresh one second before the TTL and stale one second after it.
TEST(QuoteCache, FreshWithinTheTtlAndStaleAfter) {
  QuoteCache cache(60s);
  const QuoteCache::TimePoint start{};  // a fixed reference point
  cache.store(1500, start);

  EXPECT_EQ(cache.get(start + 59s), 1500);          // inside the TTL
  EXPECT_EQ(cache.get(start + 61s), std::nullopt);  // past the TTL
}

TEST(QuoteCache, ExactlyAtTheTtlIsStale) {
  QuoteCache cache(60s);
  const QuoteCache::TimePoint start{};
  cache.store(1500, start);
  EXPECT_EQ(cache.get(start + 60s), std::nullopt);  // boundary: TTL is exclusive
}

TEST(QuoteCache, ReturnsNulloptBeforeAnythingIsStored) {
  const QuoteCache cache(60s);
  EXPECT_EQ(cache.get(QuoteCache::TimePoint{} + 1s), std::nullopt);
}

// The engine is seeded, so the choice is reproducible: a given seed always picks
// the same carrier. This is what makes a randomized unit testable at all.
TEST(ChooseBackupCarrier, IsReproducibleForAFixedSeed) {
  const std::vector<std::string> candidates{"north", "south", "east", "west"};
  std::mt19937 engine(12345);
  const std::string first = shipping::choose_backup_carrier(candidates, engine);

  std::mt19937 same_seed(12345);
  const std::string again = shipping::choose_backup_carrier(candidates, same_seed);
  EXPECT_EQ(first, again);
}

// With a known seed, the exact choice is fixed and can be asserted outright.
TEST(ChooseBackupCarrier, PicksTheExpectedCandidateForSeed12345) {
  const std::vector<std::string> candidates{"north", "south", "east", "west"};
  std::mt19937 engine(12345);
  EXPECT_EQ(shipping::choose_backup_carrier(candidates, engine), "west");
}

TEST(ChooseBackupCarrier, AlwaysReturnsOneOfTheCandidates) {
  const std::vector<std::string> candidates{"north", "south", "east", "west"};
  std::mt19937 engine(999);
  const std::string choice = shipping::choose_backup_carrier(candidates, engine);
  EXPECT_NE(std::ranges::find(candidates, choice), candidates.end());
}

}  // namespace
