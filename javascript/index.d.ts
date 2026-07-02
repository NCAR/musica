/**
 * Get MUSICA version
 * @returns {Promise<string>}
 */
export function getVersion(): Promise<string>;
/**
 * Get MICM version
 * @returns {Promise<string>}
 */
export function getMicmVersion(): Promise<string>;
export const MICM: typeof micm.MICM;
export const State: typeof micm.State;
export const Conditions: typeof micm.Conditions;
export const SolverType: {
    rosenbrock: number;
    rosenbrock_standard_order: number;
    backward_euler: number;
    backward_euler_standard_order: number;
};
export const SolverState: {
    NotYetCalled: number;
    Running: number;
    Converged: number;
    ConvergenceExceededMaxSteps: number;
    StepSizeTooSmall: number;
    RepeatedlySingularMatrix: number;
    NaNDetected: number;
    InfDetected: number;
    AcceptingUnconvergedIntegration: number;
};
export const SolverStats: typeof micm.SolverStats;
export const SolverResult: typeof micm.SolverResult;
export const RosenbrockSolverParameters: typeof micm.RosenbrockSolverParameters;
export const BackwardEulerSolverParameters: typeof micm.BackwardEulerSolverParameters;
export const GAS_CONSTANT: number;
export const AVOGADRO: 6.02214076e+23;
export const BOLTZMANN: 1.380649e-23;
export * as mechanismConfiguration from "./mechanism_configuration/index.js";
import * as micm from './micm/index.js';
import { initModule } from './backend.js';
import { getBackend } from './backend.js';
export { initModule, getBackend };
