export namespace SolverState {
    let NotYetCalled: number;
    let Running: number;
    let Converged: number;
    let ConvergenceExceededMaxSteps: number;
    let StepSizeTooSmall: number;
    let RepeatedlySingularMatrix: number;
    let NaNDetected: number;
    let InfDetected: number;
    let AcceptingUnconvergedIntegration: number;
}
/**
 * Class representing solver statistics
 */
export class SolverStats {
    constructor({ function_calls, jacobian_updates, number_of_steps, accepted, rejected, decompositions, solves, final_time, }?: {
        function_calls?: number | undefined;
        jacobian_updates?: number | undefined;
        number_of_steps?: number | undefined;
        accepted?: number | undefined;
        rejected?: number | undefined;
        decompositions?: number | undefined;
        solves?: number | undefined;
        final_time?: number | undefined;
    });
    function_calls: number;
    jacobian_updates: number;
    number_of_steps: number;
    accepted: number;
    rejected: number;
    decompositions: number;
    solves: number;
    final_time: number;
    toString(): string;
}
/**
 * Class representing the combined solver result
 */
export class SolverResult {
    /**
     * @param {number} state - The solver state (from SolverState enum)
     * @param {SolverStats} stats - The solver statistics
     */
    constructor(state: number, stats: SolverStats);
    state: number;
    stats: SolverStats;
    toString(): string;
}
