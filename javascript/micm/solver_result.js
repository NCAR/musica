/**
 * Module for solver result types from MICM.
 * 
 * This module provides:
 * - SolverState: Enum representing the final state of the solver
 * - SolverStats: Statistics from a solver run
 * - SolverResult: Combined result containing both state and statistics
 */

/**
 * Enum representing the state of the solver after execution
 */
const SolverState = {
  NotYetCalled: 0,
  Running: 1,
  Converged: 2,
  ConvergenceExceededMaxSteps: 3,
  StepSizeTooSmall: 4,
  RepeatedlySingularMatrix: 5,
  NaNDetected: 6,
  InfDetected: 7,
  AcceptingUnconvergedIntegration: 8,
};

/**
 * Class representing solver statistics
 */
class SolverStats {
  constructor({
    function_calls = 0,
    jacobian_updates = 0,
    number_of_steps = 0,
    accepted = 0,
    rejected = 0,
    decompositions = 0,
    solves = 0,
    final_time = 0.0,
  } = {}) {
    this.function_calls = function_calls;
    this.jacobian_updates = jacobian_updates;
    this.number_of_steps = number_of_steps;
    this.accepted = accepted;
    this.rejected = rejected;
    this.decompositions = decompositions;
    this.solves = solves;
    this.final_time = final_time;
  }

  toString() {
    return `SolverStats{function_calls: ${this.function_calls}, jacobian_updates: ${this.jacobian_updates}, number_of_steps: ${this.number_of_steps}, accepted: ${this.accepted}, rejected: ${this.rejected}, decompositions: ${this.decompositions}, solves: ${this.solves}, final_time: ${this.final_time}}`;
  }
}

/**
 * Class representing the combined solver result
 */
class SolverResult {
  /**
   * @param {number} state - The solver state (from SolverState enum)
   * @param {SolverStats} stats - The solver statistics
   */
  constructor(state, stats) {
    this.state = state;
    this.stats = stats;
  }

  toString() {
    const stateName = Object.keys(SolverState).find(
      (key) => SolverState[key] === this.state
    );
    return `SolverResult{state: ${stateName}, stats: ${this.stats}}`;
  }
}

module.exports = { SolverState, SolverStats, SolverResult };
