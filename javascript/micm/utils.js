export function isScalarNumber(x) {
  return (
    (typeof x === 'number' || x instanceof Number) && !Number.isNaN(x) && !(typeof x === 'boolean')
  );
}

/** Avogadro's number (mol⁻¹) */
export const AVOGADRO = 6.02214076e23;
/** Boltzmann constant (J K⁻¹) */
export const BOLTZMANN = 1.380649e-23;
/** Universal gas constant: AVOGADRO × BOLTZMANN (J K⁻¹ mol⁻¹) */
export const GAS_CONSTANT = AVOGADRO * BOLTZMANN;
