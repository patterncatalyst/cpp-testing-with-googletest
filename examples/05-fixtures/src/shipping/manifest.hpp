#ifndef SHIPPING_MANIFEST_HPP
#define SHIPPING_MANIFEST_HPP

#include <vector>

namespace shipping {

// A manifest accumulates the parcels loaded for one truck and reports their
// count and combined weight. It is the kind of small stateful object several
// tests want to start from the same way, which is what a fixture is for.
class Manifest {
 public:
  // Add a parcel of the given weight. `weight_grams` must be positive, or the
  // call throws std::invalid_argument and the manifest is left unchanged.
  void add(int weight_grams);

  [[nodiscard]] int count() const;
  [[nodiscard]] int total_grams() const;
  [[nodiscard]] bool is_empty() const;

 private:
  std::vector<int> weights_;
};

}  // namespace shipping

#endif  // SHIPPING_MANIFEST_HPP
