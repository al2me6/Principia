#pragma once

#include "numerics/polynomial_in_monomial_basis.hpp"
#include "quantities/named_quantities.hpp"

namespace principia {
namespace numerics {
namespace _polynomial_evaluators {
namespace internal {

using namespace principia::numerics::_polynomial_in_monomial_basis;
using namespace principia::quantities::_named_quantities;


template<typename Value, typename Argument, int degree>
struct PolynomialEvaluator;

template<typename Value, typename Argument, int degree, bool allow_fma>
struct EstrinEvaluator;
template<typename Value, typename Argument, int degree, bool allow_fma>
struct HornerEvaluator;

}  // namespace internal

template<typename Value, typename Argument, int degree>
using EstrinEvaluator = internal::
    EstrinEvaluator<Value, Argument, degree, /*allow_fma=*/true>;
template<typename Value, typename Argument, int degree>
using EstrinEvaluatorWithoutFMA = internal::
    EstrinEvaluator<Value, Argument, degree, /*allow_fma=*/false>;
template<typename Value, typename Argument, int degree>
using HornerEvaluator = internal::
    HornerEvaluator<Value, Argument, degree, /*allow_fma=*/true>;
template<typename Value, typename Argument, int degree>
using HornerEvaluatorWithoutFMA = internal::
    HornerEvaluator<Value, Argument, degree, /*allow_fma=*/false>;

namespace internal {

//TODO(phl)CRTP?
template<typename Value, typename Argument, int degree>
struct PolynomialEvaluator {
  using Coefficients =
      typename PolynomialInMonomialBasis<Value, Argument, degree>::Coefficients;

  Value Evaluate(Coefficients const& coefficients,
                 Argument const& argument) const = 0;
  Derivative<Value, Argument> EvaluateDerivative(
      Coefficients const& coefficients,
      Argument const& argument) const = 0;
};


// We use FORCE_INLINE because we have to write this recursively, but we really
// want linear code.

template<typename Value, typename Argument, int degree, bool allow_fma>
struct EstrinEvaluator : public PolynomialEvaluator<Value, Argument, degree> {
  using PolynomialEvaluator::Coefficients;

  FORCE_INLINE(inline) Value Evaluate(Coefficients const& coefficients,
                                      Argument const& argument) const override;
  FORCE_INLINE(inline) Derivative<Value, Argument>
  EvaluateDerivative(Coefficients const& coefficients,
                     Argument const& argument) const override;
};

template<typename Value, typename Argument, int degree, bool allow_fma>
struct HornerEvaluator : public PolynomialEvaluator<Value, Argument, degree> {
  using PolynomialEvaluator::Coefficients;

  FORCE_INLINE(inline) Value Evaluate(Coefficients const& coefficients,
                                      Argument const& argument);
  FORCE_INLINE(inline) Derivative<Value, Argument>
  EvaluateDerivative(Coefficients const& coefficients,
                     Argument const& argument);
};

}  // namespace internal
}  // namespace _polynomial_evaluators
}  // namespace numerics
}  // namespace principia

#include "numerics/polynomial_evaluators_body.hpp"
