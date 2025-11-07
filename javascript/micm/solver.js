const SolverType = {
	rosenbrock: 1, // Vector-ordered Rosenbrock solver
	rosenbrock_standard_order: 2, // Standard-ordered Rosenbrock solver
	backward_euler: 3, // Vector-ordered BackwardEuler solver
	backward_euler_standard_order: 4, // Standard-ordered BackwardEuler solver
	// cuda_rosenbrock: 5,                 // Cuda Rosenbrock solver (for later)
};
module.exports = { SolverType };
