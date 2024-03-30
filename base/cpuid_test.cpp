#include "base/cpuid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace principia {
namespace base {

using ::testing::AnyOf;
using ::testing::Test;
using namespace principia::base::_cpuid;

class CPUIDTest : public Test {};

TEST_F(CPUIDTest, Vendor) {
  // This mostly checks that we are getting something from CPUID, since it is
  // hard to expect things from the feature flags.  This could be expanded to an
  // AnyOf as needed if the tests are run on non-Intel processors.
  EXPECT_THAT(CPUVendorIdentificationString(),
              AnyOf("AuthenticAMD", "GenuineIntel"));
}

TEST_F(CPUIDTest, CPUFeatureFlags) {
  // We require Prescott or later.
  EXPECT_TRUE(cpuid_feature_flags::FPU.IsSet());
  EXPECT_TRUE(cpuid_feature_flags::SSE.IsSet());
  EXPECT_TRUE(cpuid_feature_flags::SSE2.IsSet());
  EXPECT_TRUE(cpuid_feature_flags::SSE3.IsSet());
  // Check that we don’t always return true.
  // We are not running these tests on a Pentium III, so we do not have the
  // Processor Serial Number feature.
  EXPECT_FALSE(cpuid_feature_flags::PSN.IsSet());
}

}  // namespace base
}  // namespace principia
