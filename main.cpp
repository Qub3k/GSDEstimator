#include <iostream>
#include "MLE.hpp"

int main() {

  std::vector<int32_t> v1 {1, 2, 1, 2, 1};
  estimate_gsd(v1);

  std::vector<int32_t> v2 {3, 2, 1, 2, 1};
  estimate_gsd(v2);

  std::vector<int32_t> v3 {1, 1, 1, 1, 1};
  estimate_gsd(v3);

  std::vector<int32_t> v4 {1, 4, 1, 4, 1};
  estimate_gsd(v4);

  std::vector<int32_t> v5 {5, 4, 5, 4, 5};
  estimate_gsd(v5);

  std::vector<int32_t> v6 {5, 1, 5, 4, 5};
  estimate_gsd(v6);

  std::vector<int32_t> v7 {5, 1, 5, 1, 5};
  estimate_gsd(v7);

  std::vector<int32_t> v8 {5, 5, 5, 5, 5};
  estimate_gsd(v8);

  return 0;
}
