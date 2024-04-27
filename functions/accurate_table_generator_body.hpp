#pragma once

#include "functions/accurate_table_generator.hpp"

#include <algorithm>
#include <future>
#include <limits>
#include <thread>
#include <vector>

#include "base/for_all_of.hpp"
#include "base/thread_pool.hpp"
#include "glog/logging.h"
#include "numerics/fixed_arrays.hpp"
#include "numerics/lattices.hpp"

namespace principia {
namespace functions {
namespace _accurate_table_generator {
namespace internal {

using namespace principia::base::_for_all_of;
using namespace principia::base::_thread_pool;
using namespace principia::numerics::_fixed_arrays;
using namespace principia::numerics::_lattices;

template<std::int64_t zeroes>
bool HasDesiredZeroes(cpp_bin_float_50 const& y) {
  std::int64_t y_exponent;
  auto const y_mantissa = frexp(y, &y_exponent);
  auto const y_mantissa_scaled =
      ldexp(y_mantissa, std::numeric_limits<double>::digits);
  auto const y_post_mantissa = y_mantissa_scaled - floor(y_mantissa_scaled);
  auto const y_candidate_zeroes = ldexp(y_post_mantissa, zeroes);
  return trunc(y_candidate_zeroes) == 0;
}

template<std::int64_t zeroes>
std::vector<cpp_rational> ExhaustiveMultisearch(
    std::vector<AccurateFunction> const& functions,
    std::vector<cpp_rational> const& starting_arguments) {
  ThreadPool<cpp_rational> search_pool(std::thread::hardware_concurrency());

  std::vector<std::future<cpp_rational>> futures;
  for (std::int64_t i = 0; i < starting_arguments.size(); ++i) {
    futures.push_back(search_pool.Add([i, &functions, &starting_arguments]() {
      return ExhaustiveSearch<zeroes>(functions, starting_arguments[i]);
    }));
  }

  std::vector<cpp_rational> results;
  for (auto& future : futures) {
    results.push_back(future.get());
  }
  return results;
}

template<std::int64_t zeroes>
cpp_rational ExhaustiveSearch(std::vector<AccurateFunction> const& functions,
                              cpp_rational const& starting_argument) {
  CHECK_LT(0, starting_argument);

  // We will look for candidates both above and below |starting_argument|.
  // Note that if |starting_argument| is a power of 2, the increments above
  // and below |starting_argument| are not the same.
  std::int64_t exponent;
  auto const starting_mantissa =
      frexp(static_cast<cpp_bin_float_50>(starting_argument), &exponent);
  cpp_rational const high_increment =
      exp2(exponent - std::numeric_limits<double>::digits);
  cpp_rational const low_increment =
      starting_mantissa == 0.5 ? high_increment / 2 : high_increment;

  cpp_rational high_x = starting_argument;
  cpp_rational low_x = starting_argument - low_increment;
  for (;;) {
    if (std::all_of(functions.begin(), functions.end(),
                    [&high_x](AccurateFunction const& f) {
                      return HasDesiredZeroes<zeroes>(f(high_x));
                    })) {
      return high_x;
    }
    high_x += high_increment;
    if (std::all_of(functions.begin(), functions.end(),
                    [&low_x](AccurateFunction const& f) {
                      return HasDesiredZeroes<zeroes>(f(low_x));
                    })) {
      return low_x;
    }
    low_x -= low_increment;
  }
}

#if 1
template<std::int64_t zeroes>
cpp_rational SimultaneousBadCaseSearch(
  std::array<AccurateFunction, 2> const& functions,
  std::array<AccuratePolynomial<2>, 2> const& polynomials,
  std::int64_t const M,
  std::int64_t const T) {
  auto const& F = functions;
  auto const& P = polynomials;

  cpp_rational const T_increment = cpp_rational(T) / 100;
  cpp_bin_float_50 ε = 0;
  for (std::int64_t i = 0; i < 2; ++i) {
    for (cpp_rational t = -T; t <= T; t += T_increment) {
      ε = std::max(ε, abs(F[i](t) - static_cast<cpp_bin_float_50>(P[i](t))));
    }
  }

  auto const Mʹ = static_cast<std::int64_t>(floor(M / (2 + 2 * M * ε)));
  auto const C = 3 * Mʹ;
  std::array<std::unique_ptr<AccuratePolynomial<2>>, 2> P̃;
  AccuratePolynomial<1> const Tτ({cpp_rational(0), cpp_rational(T)});
  for (std::int64_t i = 0; i < 2; ++i) {
    auto P̃_coefficients = Compose(C * P[i], Tτ).coefficients();
    for_all_of(P̃_coefficients).loop([](auto const& coefficient) {
      coefficient = round(coefficient);
    });
    P̃[i] = std::make_unique<AccuratePolynomial<2>>(P̃_coefficients);
  }

  auto const& P̃₀_coefficients = P̃[0]->coefficients();
  auto const& P̃₁_coefficients = P̃[1]->coefficients();
  FixedMatrix<cpp_rational, 5, 4> const L(
      {C, 0, std::get<0>(P̃₀_coefficients), std::get<0>(P̃₁_coefficients),
       0, C, std::get<1>(P̃₀_coefficients), std::get<1>(P̃₁_coefficients),
       0, 0, std::get<2>(P̃₀_coefficients), std::get<2>(P̃₁_coefficients),
       0, 0,                            3,                            0,
       0, 0,                            0,                            3});

  auto const V = LenstraLenstraLovász(L);
}
#endif

}  // namespace internal
}  // namespace _accurate_table_generator
}  // namespace functions
}  // namespace principia
