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
    constructor({ relative_tolerance, absolute_tolerances, h_min, h_max, h_start, max_number_of_steps, }?: {
        relative_tolerance?: number | undefined;
        absolute_tolerances?: number[] | null | undefined;
        h_min?: number | undefined;
        h_max?: number | undefined;
        h_start?: number | undefined;
        max_number_of_steps?: number | undefined;
    } | undefined);
    relative_tolerance: number;
    absolute_tolerances: number[] | null;
    h_min: number;
    h_max: number;
    h_start: number;
    max_number_of_steps: number;
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
     * @param {number[]} [options.time_step_reductions=[0.5, 0.5, 0.5, 0.5, 0.1]] Must have exactly 5 elements
     */
    constructor({ relative_tolerance, absolute_tolerances, max_number_of_steps, time_step_reductions, }?: {
        relative_tolerance?: number | undefined;
        absolute_tolerances?: number[] | null | undefined;
        max_number_of_steps?: number | undefined;
        time_step_reductions?: number[] | undefined;
    } | undefined);
    relative_tolerance: number;
    absolute_tolerances: number[] | null;
    max_number_of_steps: number;
    time_step_reductions: number[];
}
