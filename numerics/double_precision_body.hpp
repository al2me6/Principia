﻿
#pragma once

#include "numerics/double_precision.hpp"

#include <cmath>

#include "geometry/serialization.hpp"
#include "quantities/si.hpp"

namespace principia {
namespace numerics {
namespace internal_double_precision {

using geometry::PointOrMultivectorSerializer;
using geometry::QuantityOrMultivectorSerializer;

template<typename T>
constexpr DoublePrecision<T>::DoublePrecision(T const& value)
    : value(value),
      error() {}

template<typename T>
DoublePrecision<T>& DoublePrecision<T>::operator+=(
    Difference<T> const& right) {
  // See Higham, Accuracy and Stability of Numerical Algorithms, Algorithm 4.2.
  // This is equivalent to |QuickTwoSum(value, increment + error)|.
  T const temp = value;
  Difference<T> const y = increment + error;
  value = temp + y;
  error = (temp - value) + y;
}

template<typename T>
DoublePrecision<T>& DoublePrecision<T>::operator+=(
    DoublePrecision<Difference<T>> const& right) {
  *this = *this + right;
  return *this;
}

template<typename T>
DoublePrecision<T>& DoublePrecision<T>::operator-=(
    Difference<T> const& right) {
  *this = *this - right;
  return *this;
}

template<typename T>
DoublePrecision<T>& DoublePrecision<T>::operator-=(
    DoublePrecision<Difference<T>> const& right) {
  *this += (-right);
}

template<typename T>
void DoublePrecision<T>::WriteToMessage(
    not_null<serialization::DoublePrecision*> const message) const {
  using ValueSerializer = PointOrMultivectorSerializer<
                              T, serialization::DoublePrecision::Value>;
  using ErrorSerializer = QuantityOrMultivectorSerializer<
                              Difference<T>,
                              serialization::DoublePrecision::Error>;
  ValueSerializer::WriteToMessage(value, message->mutable_value());
  ErrorSerializer::WriteToMessage(error, message->mutable_error());
}

template<typename T>
DoublePrecision<T> DoublePrecision<T>::ReadFromMessage(
    serialization::DoublePrecision const& message) {
  using ValueSerializer = PointOrMultivectorSerializer<
                              T, serialization::DoublePrecision::Value>;
  using ErrorSerializer = QuantityOrMultivectorSerializer<
                              Difference<T>,
                              serialization::DoublePrecision::Error>;
  DoublePrecision double_precision;
  double_precision.value = ValueSerializer::ReadFromMessage(message.value());
  double_precision.error = ErrorSerializer::ReadFromMessage(message.error());
  return double_precision;
}

template<typename T, typename U>
DoublePrecision<Product<T, U>> Scale(T const & scale,
                                     DoublePrecision<U> const& right) {
#ifdef _DEBUG
  double const s = scale / quantities::SIUnit<T>();
  if (scale != 0.0) {
    int exponent;
    double const mantissa = std::frexp(scale, &exponent);
    CHECK_EQ(0.5, std::fabs(mantissa)) << scale;
  }
#endif
  DoublePrecision result;
  result.value = right.value * scale;
  result.error = right.error * scale;
  return result;
}

template<typename T, typename U>
DoublePrecision<Sum<T, U>> QuickTwoSum(T const& a, U const& b) {
  DCHECK_GE(std::abs(a), std::abs(b));
  // Library for Double-Double and Quad-Double Arithmetic, Hida, Li and Bailey
  // 2007.
  DoublePrecision<Sum<T, U>> result;
  auto& s = result.value;
  auto& e = result.error;
  s = a + b;
  e = b - (s - a);
  return result;
}

template<typename T, typename U>
DoublePrecision<Sum<T, U>> TwoSum(T const& a, U const& b) {
  // Library for Double-Double and Quad-Double Arithmetic, Hida, Li and Bailey
  // 2007.
  DoublePrecision<Sum<T, U>> result;
  auto& s = result.value;
  auto& e = result.error;
  s = a + b;
  auto const v = s - a;
  e = (a - (s - v)) + (b - v);
  return result;
}

template<typename T>
DoublePrecision<Difference<T>> operator+(
    DoublePrecision<Difference<T>> const& left) {
  return left;
}

template<typename T>
DoublePrecision<Difference<T>> operator-(
    DoublePrecision<Difference<T>> const& left) {
  DoublePrecision<Difference<T>> result;
  result.value = -left.value;
  result.error = -left.error;
  return result;
}

template<typename T, typename U>
DoublePrecision<Sum<T, U>> operator+(DoublePrecision<T> const& left,
                                     DoublePrecision<U> const& right) {
  // Software for Doubled-Precision Floating-Point Computations, Linnainmaa,
  // 1981, algorithm longadd.
  auto const sum = TwoSum(left.value, right.value);
  return QuickTwoSum(sum.value, (sum.error + left.error) + right.error);
}

template<typename T, typename U>
DoublePrecision<Difference<T, U>> operator-(DoublePrecision<T> const& left,
                                            DoublePrecision<U> const& right) {
  return left + (-right);
}

template<typename T>
std::ostream& operator<<(std::ostream& os,
                         const DoublePrecision<T>& double_precision) {
  os << double_precision.value << "|" << double_precision.error;
  return os;
}

}  // namespace internal_double_precision
}  // namespace numerics
}  // namespace principia
