#pragma once

#include <functional>
#include <list>
#include <map>
#include <memory>

#include "geometry/grassmann.hpp"
#include "physics/degrees_of_freedom.hpp"
#include "quantities/named_quantities.hpp"

using principia::geometry::Vector;
using principia::quantities::Acceleration;
using principia::quantities::Length;
using principia::quantities::Speed;
using principia::quantities::Time;

namespace principia {
namespace physics {

class Body;

template<typename Frame>
class Trajectory {
 public:
  // No transfer of ownership.  |body| must live longer than the trajectory as
  // the trajectory holds a reference to it.
  explicit Trajectory(Body const& body);
  ~Trajectory() = default;

  // These functions return the series of positions/velocities/times for the
  // trajectory of the body.  All three containers are guaranteed to have the
  // same size.  These functions are O(|depth| + |length|).
  std::map<Point<Time>, Point<Vector<Length, Frame>>> Positions() const;
  std::map<Point<Time>, Vector<Speed, Frame>> Velocities() const;
  std::list<Point<Time>> Times() const;

  // Return the most recent position/velocity/time.  These functions are O(1)
  // and dirt-cheap.
  Point<Vector<Length, Frame>> const& last_position() const;
  Vector<Speed, Frame> const& last_velocity() const;
  Point<Time> const& last_time() const;

  // Appends one point to the trajectory.
  void Append(Point<Time> const& time,
              DegreesOfFreedom<Frame> const& degrees_of_freedom);

  // Removes all data for times (strictly) greater than |time|, as well as all
  // child trajectories forked at times (strictly) greater than |time|.
  void ForgetAfter(Point<Time> const& time);

  // Removes all data for times less than or equal to |time|, as well as all
  // child trajectories forked at times less than or equal to |time|.  This
  // trajectory must be a root.
  void ForgetBefore(Point<Time> const& time);

  // Creates a new child trajectory forked at time |time|, and returns it.  The
  // child trajectory may be changed independently from the parent trajectory
  // for any time (strictly) greater than |time|.  The child trajectory is owned
  // by its parent trajectory.  Calling ForgetAfter or ForgetBefore on the
  // parent trajectory with an argument that causes the time |time| to be
  // removed deletes the child trajectory.  Deleting the parent trajectory
  // deletes all child trajectories.  |time| must be one of the times of the
  // current trajectory (as returned by Times()).  No transfer of ownership.
  Trajectory* Fork(Point<Time> const& time);

  // Returns true if this is a root trajectory.
  bool is_root() const;

  // Returns the root trajectory.
  Trajectory const* root() const;
  Trajectory* root();

  // Returns the fork time for a nonroot trajectory and null for a root
  // trajectory.
  Point<Time> const* fork_time() const;

  // The body to which this trajectory pertains.
  Body const& body() const;

  // This function represents the intrinsic acceleration of a body, irrespective
  // of any external field.  It can be due e.g., to an engine burn.
  typedef std::function<Vector<Acceleration, Frame>(Point<Time> const& time)>
      IntrinsicAcceleration;

  // Sets the intrinsic acceleration for the trajectory of a massless body.
  // For a nonroot trajectory the intrinsic acceleration only applies to times
  // (strictly) greater than |fork_time()|.  In other words, the function
  // |acceleration| is never called with times less than or equal to
  // |fork_time()|.  It may, however, be called with times beyond |last_time()|.
  // For a root trajectory the intrinsic acceleration applies to times greater
  // than or equal to the first time of the trajectory.  Again, it may apply
  // beyond |last_time()|.
  // It is an error to call this function for a trajectory that already has an
  // intrinsic acceleration, or for the trajectory of a massive body.
  void set_intrinsic_acceleration(IntrinsicAcceleration&& acceleration);

  // Removes any intrinsic acceleration for the trajectory.
  void clear_intrinsic_acceleration();

  // Returns true if this trajectory has an intrinsic acceleration.
  bool has_intrinsic_acceleration() const;

  // Computes the intrinsic acceleration for this trajectory at time |time|.  If
  // |has_intrinsic_acceleration()| return false, or if |time| is before the
  // |fork_time()| (or initial time) of this trajectory, the returned
  // acceleration is zero.
  Vector<Acceleration, Frame> evaluate_intrinsic_acceleration(
      Point<Time> const& time) const;

 private:
  typedef std::map<Point<Time>, DegreesOfFreedom<Frame>> Timeline;

  // A constructor for creating a child trajectory during forking.
  Trajectory(Body const& body,
             Trajectory* const parent,
             typename Timeline::iterator const& fork);

  template<typename Value>
  std::map<Point<Time>, Value> ApplyToDegreesOfFreedom(
      std::function<Value(DegreesOfFreedom<Frame> const&)> compute_value) const;

  Body const& body_;

  Trajectory* const parent_;  // Null for a root trajectory.

  // Null for a root trajectory.
  std::unique_ptr<typename Timeline::iterator> fork_;

  // There may be several forks starting from the same time, hence the multimap.
  // Child trajectories are owned.
  std::multimap<Point<Time>, std::unique_ptr<Trajectory>> children_;

  Timeline timeline_;

  std::unique_ptr<IntrinsicAcceleration> intrinsic_acceleration_;
};

}  // namespace physics
}  // namespace principia

#include "trajectory_body.hpp"
