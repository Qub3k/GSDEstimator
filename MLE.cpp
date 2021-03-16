#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>

template<typename T>
void print_vector(std::vector<T> &vec) {
  std::cout << "{";
  for( auto el: vec ) {
    std::cout << el << " ";
  }
  std::cout << "}";
}

std::pair<float, float> estimate_gsd(std::vector<int32_t> &input) {

  // Step 1
  std::sort(std::begin(input), std::end(input));
  if (*(std::end(input)-1) - *(std::begin(input)) == 1) {
    std::cout << "Executing Step 1 on ";
    print_vector(input);
    std::cout << std::endl;

    int32_t k = *std::begin(input);
    int32_t kp1 = k + 1;
    int32_t n = input.size();
    int32_t nk = std::distance(
        std::begin(input),
        std::upper_bound(std::begin(input), std::end(input), k));
    int32_t nkp1 = n - nk;
    return std::make_pair((k*nk + kp1*nkp1)/n, 1);

  }

  // Step 2
  auto first_not_1 = std::upper_bound(std::begin(input), std::end(input), 1);
  if (first_not_1 != std::end(input) && *first_not_1 == 5) {

    std::cout << "Executing Step 2 on ";
    print_vector(input);
    std::cout << std::endl;
    int32_t n  = input.size();
    int32_t n1 = std::distance(std::begin(input), first_not_1);
    int32_t n5 = n - n1;
    return std::make_pair((n1 + n5)/n, 0);

  }

  std::cout << "Executing Step 3 on ";
  print_vector(input);
  std::cout << std::endl;

  return std::make_pair(0, 0);
}
