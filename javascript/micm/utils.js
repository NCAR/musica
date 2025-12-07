function isScalarNumber(x) {
	return (
		(typeof x === 'number' || x instanceof Number) &&
		!Number.isNaN(x) &&
		!(typeof x === 'boolean')
	);
}

const AVOGADRO = 6.02214076e23  // mol^-1
const BOLTZMANN = 1.380649e-23  // J K^-1
const GAS_CONSTANT = AVOGADRO * BOLTZMANN  // J K^-1 mol^-1

module.exports = { isScalarNumber, AVOGADRO, BOLTZMANN, GAS_CONSTANT };
