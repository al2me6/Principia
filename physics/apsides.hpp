#pragma once

#include <functional>

#include "absl/status/status.h"
#include "base/constant_function.hpp"
#include "physics/discrete_traject0ry.hpp"
#include "physics/trajectory.hpp"

namespace principia {
namespace physics {
namespace internal_apsides {

using base::ConstantFunction;
using base::Identically;
using geometry::Vector;

// Computes the apsides with respect to |reference| for the section given by
// |begin| and |end| of |trajectory|.  Appends to the given output trajectories
// one point for each apsis.
template<typename Frame>
void ComputeApsides(Trajectory<Frame> const& reference,
                    Trajectory<Frame> const& trajectory,
                    typename DiscreteTraject0ry<Frame>::iterator begin,
                    typename DiscreteTraject0ry<Frame>::iterator end,
                    int max_points,
                    DiscreteTraject0ry<Frame>& apoapsides,
                    DiscreteTraject0ry<Frame>& periapsides);

// Computes the crossings of the section given by |begin| and |end| of
// |trajectory| with the xy plane.  Appends the crossings that go towards the
// |north| side of the xy plane to |ascending|, and those that go away from the
// |north| side to |descending|.
// Nodes for which |predicate| returns false are excluded.
template<typename Frame, typename Predicate = ConstantFunction<bool>>
absl::Status ComputeNodes(Trajectory<Frame> const& trajectory,
                          typename DiscreteTraject0ry<Frame>::iterator begin,
                          typename DiscreteTraject0ry<Frame>::iterator end,
                          Vector<double, Frame> const& north,
                          int max_points,
                          DiscreteTraject0ry<Frame>& ascending,
                          DiscreteTraject0ry<Frame>& descending,
                          Predicate predicate = Identically(true));

// TODO(egg): when we can usefully iterate over an arbitrary |Trajectory|, move
// the following from |Ephemeris|.
#if 0
template<typename Frame>
void ComputeApsides(Trajectory<Frame> const& trajectory1,
                    Trajectory<Frame> const& trajectory2,
                    DiscreteTraject0ry<Frame>& apoapsides1,
                    DiscreteTraject0ry<Frame>& periapsides1,
                    DiscreteTraject0ry<Frame>& apoapsides2,
                    DiscreteTraject0ry<Frame>& periapsides2);
#endif

}  // namespace internal_apsides

using internal_apsides::ComputeApsides;
using internal_apsides::ComputeNodes;

}  // namespace physics
}  // namespace principia

#include "physics/apsides_body.hpp"
