#ifndef SHIPPING_PARCEL_HPP
#define SHIPPING_PARCEL_HPP

namespace shipping {

// The outer dimensions of a parcel, in centimetres. Assumed positive.
struct Dimensions {
  int length_cm;
  int width_cm;
  int height_cm;
};

// Carriers bill the greater of a parcel's actual weight and its volumetric
// (dimensional) weight, so a large, light box is not shipped for the price of
// its scale weight alone. Volumetric weight in grams is the box volume in cubic
// centimetres divided by 5 (the common 5000 cm3-per-kg divisor). `actual_grams`
// is the scale weight; both it and the dimensions are assumed positive.
[[nodiscard]] int billable_weight_grams(Dimensions box, int actual_grams);

}  // namespace shipping

#endif  // SHIPPING_PARCEL_HPP
