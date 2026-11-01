#include "shipping/manifest.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace {

// The fixture: every TEST_F below runs against a freshly constructed
// ManifestTest, so SetUp's two parcels are the starting point for each one and
// no test can see another test's changes.
class ManifestTest : public ::testing::Test {
 protected:
  void SetUp() override {
    manifest_.add(1000);
    manifest_.add(2500);
  }

  // A helper the test bodies can call directly, because each TEST_F body runs as
  // a method of the fixture. It keeps repetitive setup out of the cases.
  void add_parcels(int how_many, int each_grams) {
    for (int i = 0; i < how_many; ++i) {
      manifest_.add(each_grams);
    }
  }

  shipping::Manifest manifest_;
};

TEST_F(ManifestTest, StartsFromTheParcelsAddedInSetUp) {
  EXPECT_EQ(manifest_.count(), 2);
  EXPECT_EQ(manifest_.total_grams(), 3500);
  EXPECT_FALSE(manifest_.is_empty());
}

TEST_F(ManifestTest, AddingAParcelUpdatesCountAndTotal) {
  manifest_.add(500);
  EXPECT_EQ(manifest_.count(), 3);
  EXPECT_EQ(manifest_.total_grams(), 4000);
}

TEST_F(ManifestTest, EachTestStartsFromAFreshFixture) {
  // Also adds 500. If fixture state leaked between tests, the count would climb
  // past 3; it does not, which proves each test gets its own manifest.
  manifest_.add(500);
  EXPECT_EQ(manifest_.count(), 3);
}

TEST_F(ManifestTest, AFixtureHelperKeepsRepetitiveSetupOutOfTheCase) {
  add_parcels(3, 100);
  EXPECT_EQ(manifest_.count(), 5);            // 2 from SetUp + 3 from the helper
  EXPECT_EQ(manifest_.total_grams(), 3800);   // 3500 + 300
}

TEST_F(ManifestTest, RejectsANonPositiveWeightAndLeavesTheManifestUnchanged) {
  EXPECT_THROW(manifest_.add(0), std::invalid_argument);
  EXPECT_EQ(manifest_.count(), 2);
}

}  // namespace
