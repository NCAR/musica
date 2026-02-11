/**
 * Parameters for configuring Rosenbrock solvers.
 */
export class RosenbrockSolverParameters {
  /**
   * @param {Object} [options]
   * @param {number} [options.relative_tolerance=1e-6]
   * @param {number[]|null} [options.absolute_tolerances=null]
   * @param {number} [options.h_min=0.0]
   * @param {number} [options.h_max=0.0]
   * @param {number} [options.h_start=0.0]
   * @param {number} [options.max_number_of_steps=1000]
   */
  constructor({
    relative_tolerance = 1e-6,
    absolute_tolerances = null,
    h_min = 0.0,
    h_max = 0.0,
    h_start = 0.0,
    max_number_of_steps = 1000,
  } = {}) {
    this.relative_tolerance = relative_tolerance;
    this.absolute_tolerances = absolute_tolerances;
    this.h_min = h_min;
    this.h_max = h_max;
    this.h_start = h_start;
    this.max_number_of_steps = max_number_of_steps;
  }
}

/**
 * Parameters for configuring Backward Euler solvers.
 */
export class BackwardEulerSolverParameters {
  /**
   * @param {Object} [options]
   * @param {number} [options.relative_tolerance=1e-6]
   * @param {number[]|null} [options.absolute_tolerances=null]
   * @param {number} [options.max_number_of_steps=11]
   * @param {number[]} [options.time_step_reductions=[0.5, 0.5, 0.5, 0.5, 0.1]]
   */
  constructor({
    relative_tolerance = 1e-6,
    absolute_tolerances = null,
    max_number_of_steps = 11,
    time_step_reductions = [0.5, 0.5, 0.5, 0.5, 0.1],
  } = {}) {
    this.relative_tolerance = relative_tolerance;
    this.absolute_tolerances = absolute_tolerances;
    this.max_number_of_steps = max_number_of_steps;
    this.time_step_reductions = time_step_reductions;
  }
}
