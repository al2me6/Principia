﻿#include "physics/discrete_trajectory2.hpp"

#include "geometry/frame.hpp"
#include "geometry/named_quantities.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "quantities/si.hpp"
#include "testing_utilities/discrete_trajectory_factories.hpp"

namespace principia {
namespace physics {

using geometry::Frame;
using geometry::Instant;
using geometry::Velocity;
using quantities::si::Metre;
using quantities::si::Second;
using testing_utilities::NewLinearTrajectoryTimeline;

class DiscreteTrajectory2Test : public ::testing::Test {
 protected:
  using World = Frame<enum class WorldTag>;

  DiscreteTrajectory2Test() {
    // Construct a trajectory with three 10-second segments.
    std::optional<DegreesOfFreedom<World>> last_degrees_of_freedom;

    Velocity<World> const v1({1 * Metre / Second,
                              0 * Metre / Second,
                              0 * Metre / Second});
    for (auto const& [t, degrees_of_freedom] :
         NewLinearTrajectoryTimeline(v1,
                                     /*Δt=*/1 * Second,
                                     /*t1=*/t0_,
                                     /*t2=*/t0_ + 10 * Second)) {
      last_degrees_of_freedom = degrees_of_freedom;
      trajectory_.Append(t, degrees_of_freedom);
    }

    trajectory_.NewSegment();
    Velocity<World> const v2({0 * Metre / Second,
                              1 * Metre / Second,
                              0 * Metre / Second});
    for (auto const& [t, degrees_of_freedom] :
        NewLinearTrajectoryTimeline(DegreesOfFreedom<World>(
                                        last_degrees_of_freedom->position(),
                                        v2),
                                     /*Δt=*/1 * Second,
                                     /*t1=*/t0_ + 10 * Second,
                                     /*t2=*/t0_ + 20 * Second)) {
      last_degrees_of_freedom = degrees_of_freedom;
      trajectory_.Append(t, degrees_of_freedom);
    }

    trajectory_.NewSegment();
    Velocity<World> const v3({0 * Metre / Second,
                              0 * Metre / Second,
                              1 * Metre / Second});
    for (auto const& [t, degrees_of_freedom] :
        NewLinearTrajectoryTimeline(DegreesOfFreedom<World>(
                                        last_degrees_of_freedom->position(),
                                        v3),
                                     /*Δt=*/1 * Second,
                                     /*t1=*/t0_ + 20 * Second,
                                     /*t2=*/t0_ + 30 * Second)) {
      trajectory_.Append(t, degrees_of_freedom);
    }
  }

  Instant const t0_;
  DiscreteTrajectory2<World> trajectory_;
};

TEST_F(DiscreteTrajectory2Test, No) {

}

}  // namespace physics
}  // namespace principia
