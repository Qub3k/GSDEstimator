#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cmath>

template<typename T>
void print_vector(std::vector<T> &vec) {
  std::cout << "{";
  for( auto el: vec ) {
    std::cout << el << " ";
  }
  std::cout << "}";
}

// likelihood function for beta-binomial distribution
float lBB(std::vector<int32_t> &input, float alpha, float beta) {
  float result = 0;
  for (int32_t i = 0; i < 4; ++i) {
    result += std::log(alpha + beta + i);
  }
  result *= -1*(float)input.size();

  auto last = std::begin(input);
  auto end = std::end(input);
  for (int32_t k = 0; k < 5 && last != end; ++k) {
    auto bound = std::upper_bound(last, std::end(input), k+1);
    int32_t nkp1 = std::distance(last, bound);
    if (nkp1 == 0)
      continue;
    last = bound;

    float partial = 0;
    for (int32_t i = 0; i < k; ++i) {
      partial += std::log(alpha + i);
    }
    for (int32_t i = 0; i < 4-k; ++i) {
      partial += std::log(beta + i);
    }
    result += nkp1 * partial;
  }

  return result;
}

std::pair<float, float> estimate_gsd(std::vector<int32_t> &input) {

  // Step 1
  std::sort(std::begin(input), std::end(input));
  if (*(std::end(input)-1) - *(std::begin(input)) == 1) {
#ifdef DEBUG
    std::cout << "Executing Step 1 on ";
    print_vector(input);
    std::cout << std::endl;
#endif

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

#ifdef DEBUG
    std::cout << "Executing Step 2 on ";
    print_vector(input);
    std::cout << std::endl;
#endif
    int32_t n  = input.size();
    int32_t n1 = std::distance(std::begin(input), first_not_1);
    int32_t n5 = n - n1;
    return std::make_pair((n1 + n5)/n, 0);

  }

#ifdef DEBUG
  std::cout << "Executing Step 3 on ";
  print_vector(input);
  std::cout << std::endl;
#endif

  float a = 1;
  float b;
  float step = 0.1;
  while (a < 200) {
    b = 1;
    while (b < 200) {
      std::cout << lBB(input, a, b) << std::endl;
      b += step;
    }
    a += step;
  }


  return std::make_pair(0, 0);
}
