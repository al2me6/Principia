#pragma once

#include "integrators/starter.hpp"

namespace principia {
namespace integrators {
namespace internal_starter {

constexpr int startup_step_divisor = 16;

template<typename ODE, int order>
Starter<ODE, order>::Starter(
    FixedStepSizeIntegrator<ODE> const& startup_integrator,
    not_null<typename FixedStepSizeIntegrator<ODE>::Instance*> const instance)
    : startup_integrator_(startup_integrator),
      instance_(instance) {}

template<typename ODE, int order>
void Starter<ODE, order>::StartupSolve(IndependentVariable const& s_final) {
  auto const& equation = instance_->equation();
  auto& current_state = instance_->state();
  auto const& step = instance_->step();

  IndependentVariable const startup_step = step / startup_step_divisor;

  CHECK(!previous_steps_.empty());
  CHECK_LT(previous_steps_.size(), order);

  auto const startup_append_state =
      [&current_state, &equation](typename ODE::SystemState const& state) {
        // Stop changing anything once we're done with the startup.  We may be
        // called one more time by the |startup_integrator_|.
        if (previous_steps_.size() < order) {
          current_state = state;
          // The startup integrator has a smaller step.  We do not record all
          // the states it computes, but only those that are a multiple of the
          // main integrator step.
          if (++startup_step_index_ % startup_step_divisor == 0) {
            CHECK_LT(previous_steps_.size(), order);
            previous_steps_.emplace_back();
            FillStepFromSystemState(equation,
                                    current_state,
                                    previous_steps_.back());
            // This call must happen last for a subtle reason: the callback may
            // want to |Clone| this instance (see |Ephemeris::Checkpoint|) in
            // which cases it is necessary that all the member variables be
            // filled for restartability to work.
            this->append_state_(state);
          }
        }
      };

  auto const startup_instance =
      startup_integrator_.NewInstance({equation, current_state},
                                       startup_append_state,
                                       startup_step);

  startup_instance->Solve(
      std::min(instance_->time().value +
                   (order - previous_steps_.size()) * step + step / 2.0,
               s_final)).IgnoreError();

  CHECK_LE(previous_steps_.size(), order);
}

template<typename ODE, int order>
void Starter<ODE, order>::FillStepFromSystemState(
    ODE const& equation,
    typename ODE::SystemState const& state,
    Step& step) {
  std::vector<DependentVariable> dependent_variables;
  step.s = state.time;
  for (auto const& position : state.positions) {
    step.displacements.push_back(position - DoublePrecision<Position>());
    dependent_variables.push_back(position.value);
  }
  step.accelerations.resize(step.displacements.size());
  // Ignore the status here.  We are merely computing the acceleration to store
  // it, not to advance an integrator.
  equation.compute_acceleration(step.time.value,
                                dependent_variables,
                                step.accelerations).IgnoreError();
}



}  // namespace internal_starter
}  // namespace integrators
}  // namespace principia
