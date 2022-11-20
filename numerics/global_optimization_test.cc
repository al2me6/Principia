#include "numerics/global_optimization.hpp"

#include "geometry/frame.hpp"
#include "geometry/grassmann.hpp"
#include "geometry/named_quantities.hpp"
#include "gtest/gtest.h"
#include "quantities/named_quantities.hpp"
#include "quantities/quantities.hpp"
#include "quantities/si.hpp"
#include "testing_utilities/almost_equals.hpp"
#include "testing_utilities/approximate_quantity.hpp"
#include "testing_utilities/componentwise.hpp"
#include "testing_utilities/is_near.hpp"
#include "testing_utilities/numerics_matchers.hpp"
#include "testing_utilities/vanishes_before.hpp"

namespace principia {
namespace numerics {

using geometry::Displacement;
using geometry::Frame;
using geometry::Position;
using geometry::Vector;
using quantities::Inverse;
using quantities::Length;
using quantities::Pow;
using quantities::si::Metre;
using testing_utilities::AbsoluteErrorFrom;
using testing_utilities::AlmostEquals;
using testing_utilities::Componentwise;
using testing_utilities::IsNear;
using testing_utilities::RelativeErrorFrom;
using testing_utilities::operator""_;
using ::testing::UnorderedElementsAre;

// The test functions in this file are from
// https://www.sfu.ca/~ssurjano/optimization.html.
class GlobalOptimizationTest : public ::testing::Test {
 protected:
  using World = Frame<enum class WorldTag>;
};

TEST_F(GlobalOptimizationTest, GoldsteinPrice) {
  using Optimizer = MultiLevelSingleLinkage<double, Displacement<World>>;
  int function_invocations = 0;
  int gradient_invocations = 0;

  auto goldstein_price = [&function_invocations](
                             Displacement<World> const& displacement) {
    ++function_invocations;
    auto const& coordinates = displacement.coordinates();
    double const x₁ = coordinates[1] / Metre;
    double const x₂ = coordinates[2] / Metre;
    return (1 + Pow<2>(x₁ + x₂ + 1) * (19 - 14 * x₁ + 3 * Pow<2>(x₁) - 14 * x₂ +
                                       6 * x₁ * x₂ + 3 * Pow<2>(x₂))) *
               (30 + Pow<2>(2 * x₁ - 3 * x₂) *
                         (18 - 32 * x₁ + 12 * Pow<2>(x₁) + 48 * x₂ -
                          36 * x₁ * x₂ + 27 * Pow<2>(x₂)));
  };

  auto grad_goldstein_price = [&gradient_invocations](
                                  Displacement<World> const& displacement) {
    ++gradient_invocations;
    auto const& coordinates = displacement.coordinates();
    double const x₁ = coordinates[1] / Metre;
    double const x₂ = coordinates[2] / Metre;
    double const g₁ =
        24 * (-1 + 2 * x₁ - 3 * x₂) * (2 * x₁ - 3 * x₂) *
            (2 * x₁ - 3 * (1 + x₂)) *
            (1 +
             Pow<2>(1 + x₁ + x₂) * (19 + 3 * Pow<2>(x₁) + x₂ * (-14 + 3 * x₂) +
                                    2 * x₁ * (-7 + 3 * x₂))) +
        12 * (-2 + x₁ + x₂) * (-1 + x₁ + x₂) * (1 + x₁ + x₂) *
            (30 + Pow<2>(2 * x₁ - 3 * x₂) *
                      (18 + 12 * Pow<2>(x₁) - 4 * x₁ * (8 + 9 * x₂) +
                       3 * x₂ * (16 + 9 * x₂)));
    double const g₂ =
        -36 * (-1 + 2 * x₁ - 3 * x₂) * (2 * x₁ - 3 * x₂) *
            (2 * x₁ - 3 * (1 + x₂)) *
            (1 +
             Pow<2>(1 + x₁ + x₂) * (19 + 3 * Pow<2>(x₁) + x₂ * (-14 + 3 * x₂) +
                                    2 * x₁ * (-7 + 3 * x₂))) +
        12 * (-2 + x₁ + x₂) * (-1 + x₁ + x₂) * (1 + x₁ + x₂) *
            (30 + Pow<2>(2 * x₁ - 3 * x₂) *
                      (18 + 12 * Pow<2>(x₁) - 4 * x₁ * (8 + 9 * x₂) +
                       3 * x₂ * (16 + 9 * x₂)));
    return Vector<Inverse<Length>, World>({0 / Metre, g₁ / Metre, g₂ / Metre});
  };

  // Correctness checks for the function and its gradient.
  {
    auto const test_point =
        Displacement<World>({0 * Metre, 0.5 * Metre, -0.3 * Metre});
    EXPECT_THAT(goldstein_price(test_point), IsNear(596.161_(1)));
    EXPECT_THAT(grad_goldstein_price(test_point),
                Componentwise(AlmostEquals(0 / Metre, 0),
                              IsNear(-601.51_(1) / Metre),
                              IsNear(2163.65_(1) / Metre)));
  }

  Optimizer::Box const box = {
      .centre = Displacement<World>(),
      .vertices = {
          Displacement<World>({0 * Metre, 0 * Metre, 0 * Metre}),
          Displacement<World>({0 * Metre, 2 * Metre, 0 * Metre}),
          Displacement<World>({0 * Metre, 0 * Metre, 2 * Metre}),
      }};

  const auto tolerance = 1e-6 * Metre;
  Optimizer optimizer(box, goldstein_price, grad_goldstein_price);

  {
    auto const minima = optimizer.FindGlobalMinima(/*points_per_round=*/20,
                                                   /*number_of_rounds=*/10,
                                                   tolerance);

    EXPECT_EQ(2740, function_invocations);
    EXPECT_EQ(1813, gradient_invocations);
    EXPECT_THAT(
        minima,
        UnorderedElementsAre(
            Componentwise(
                AbsoluteErrorFrom(0 * Metre, IsNear(7.6e-7_(1) * Metre)),
                AbsoluteErrorFrom(0 * Metre, IsNear(5.3e-8_(1) * Metre)),
                RelativeErrorFrom(-1 * Metre, IsNear(3.8e-8_(1)))),
            Componentwise(
                AbsoluteErrorFrom(0 * Metre, IsNear(5.6e-8_(1) * Metre)),
                RelativeErrorFrom(-0.6 * Metre, IsNear(6.8e-10_(1))),
                RelativeErrorFrom(-0.4 * Metre, IsNear(1.1e-9_(1)))),
            Componentwise(
                AbsoluteErrorFrom(0 * Metre, IsNear(5.6e-8_(1) * Metre)),
                RelativeErrorFrom(1.8 * Metre, IsNear(1.8e-10_(1))),
                RelativeErrorFrom(0.2 * Metre, IsNear(7.0e-10_(1))))));
  }
  {
    auto const minima =
        optimizer.FindGlobalMinima(/*points_per_round=*/500,
                                   /*number_of_rounds=*/std::nullopt,
                                   tolerance);

    EXPECT_EQ(3620, function_invocations);
    EXPECT_EQ(2474, gradient_invocations);
    EXPECT_THAT(
        minima,
        UnorderedElementsAre(
            Componentwise(
                AbsoluteErrorFrom(0 * Metre, IsNear(1.6e-7_(1) * Metre)),
                RelativeErrorFrom(-0.6 * Metre, IsNear(1.6e-8_(1))),
                RelativeErrorFrom(-0.4 * Metre, IsNear(7.0e-8_(1)))),
            Componentwise(
                AbsoluteErrorFrom(0 * Metre, IsNear(4.0e-7_(1) * Metre)),
                AbsoluteErrorFrom(0 * Metre, IsNear(1.1e-7_(1) * Metre)),
                RelativeErrorFrom(-1 * Metre, IsNear(7.0e-8_(1)))),
            Componentwise(
                AbsoluteErrorFrom(0 * Metre, IsNear(1.3e-7_(1) * Metre)),
                RelativeErrorFrom(1.2 * Metre, IsNear(6.7e-10_(1))),
                RelativeErrorFrom(0.8 * Metre, IsNear(9.6e-10_(1)))),
            Componentwise(
                AbsoluteErrorFrom(0 * Metre, IsNear(3.6e-7_(1) * Metre)),
                RelativeErrorFrom(1.8 * Metre, IsNear(4.4e-8_(1))),
                RelativeErrorFrom(0.2 * Metre, IsNear(5.2e-7_(1))))));
  }
}

}  // namespace numerics
}  // namespace principia
