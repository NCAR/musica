/**
 * Shared test utilities for JavaScript tests
 */

/**
 * Check if two values are close (equivalent to pytest.approx or np.isclose)
 * @param {number} a - First value
 * @param {number} b - Second value (reference value)
 * @param {number} atol - Absolute tolerance (default: 1e-5)
 * @param {number} rtol - Relative tolerance (default: 1e-9)
 * @returns {boolean} True if values are close within tolerance
 */
function isClose(a, b, atol = 1e-5, rtol = 1e-9) {
  const diff = Math.abs(a - b);
  return diff <= (atol + rtol * Math.abs(b));
}

module.exports = {
  isClose
};
