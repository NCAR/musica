// Frontend API Client for MUSICA Backend
// Provides methods to interact with the Express backend

const API_BASE = import.meta.env.VITE_API_URL || 'http://localhost:3001/api';

/**
 * Generic fetch wrapper with error handling
 */
async function fetchAPI(endpoint, options = {}) {
  try {
    const response = await fetch(`${API_BASE}${endpoint}`, {
      headers: {
        'Content-Type': 'application/json',
        ...options.headers,
      },
      ...options,
    });

    const data = await response.json();

    if (!response.ok) {
      throw new Error(data.error || `HTTP error! status: ${response.status}`);
    }

    return data;
  } catch (error) {
    console.error(`API Error (${endpoint}):`, error);
    throw error;
  }
}

/**
 * MUSICA API Client
 */
export const api = {
  /**
   * Health check
   * @returns {Promise<Object>} { status, timestamp }
   */
  async getHealth() {
    return fetchAPI('/health');
  },

  /**
   * Get available mechanisms
   * @returns {Promise<Object>} { success, mechanisms: [{id, name, description, species, reactions}] }
   */
  async getMechanisms() {
    return fetchAPI('/mechanisms');
  },

  /**
   * Get specific mechanism details
   * @param {string} mechanismId - Mechanism ID
   * @returns {Promise<Object>} { success, mechanism: {...} }
   */
  async getMechanism(mechanismId) {
    return fetchAPI(`/mechanisms/${mechanismId}`);
  },

  /**
   * Get species list for a specific mechanism
   * @param {string} mechanismId - Mechanism ID (ts1, chapman, etc.)
   * @returns {Promise<Object>} { success, mechanism, species: [...] }
   */
  async getSpecies(mechanismId) {
    return fetchAPI(`/mechanisms/${mechanismId}/species`);
  },

  /**
   * Run a simulation
   * @param {Object} config - Simulation configuration
   * @param {string} config.mechanism - Mechanism ID (ts1, chapman, etc.)
   * @param {number} config.temperature - Temperature in K (default: 298.15)
   * @param {number} config.pressure - Pressure in Pa (default: 101325)
   * @param {number} config.timeStep - Time step in seconds (default: 200)
   * @param {number} config.duration - Total duration in seconds (default: 3600)
   * @param {Object} config.initialConcentrations - Initial species concentrations
   * @param {Object} config.rateConstants - User-defined rate constants (photolysis, etc.)
   * @param {number} config.outputFrequency - Output every N steps (default: 10)
   * @returns {Promise<Object>} { success, simulationId, results: [...], metadata: {...} }
   */
  async runSimulation(config) {
    return fetchAPI('/simulation/run', {
      method: 'POST',
      body: JSON.stringify(config),
    });
  },

  /**
   * Get simulation results by ID
   * @param {string} simulationId - Simulation ID
   * @returns {Promise<Object>} { success, simulation: {...} }
   */
  async getSimulation(simulationId) {
    return fetchAPI(`/simulation/${simulationId}`);
  },

  /**
   * List all simulations
   * @returns {Promise<Object>} { success, simulations: [...], count }
   */
  async listSimulations() {
    return fetchAPI('/simulation');
  },

  /**
   * Delete a simulation
   * @param {string} simulationId - Simulation ID
   * @returns {Promise<Object>} { success, message }
   */
  async deleteSimulation(simulationId) {
    return fetchAPI(`/simulation/${simulationId}`, {
      method: 'DELETE',
    });
  },
};

/**
 * Helper function to format simulation results for charts
 * @param {Array} results - Simulation results from API
 * @returns {Array} Formatted data for Recharts
 */
export function formatSimulationData(results) {
  if (!results || !Array.isArray(results)) {
    return [];
  }

  return results.map((result) => {
    const { time, concentrations } = result;

    // Flatten concentrations for easier charting
    const dataPoint = { time };

    for (const [species, values] of Object.entries(concentrations)) {
      // Take first value if array (for single grid cell)
      dataPoint[species] = Array.isArray(values) ? values[0] : values;
    }

    return dataPoint;
  });
}

/**
 * Helper function to get default initial concentrations for a mechanism
 * @param {string} mechanismId - Mechanism ID
 * @returns {Object} Default concentrations
 */
export function getDefaultConcentrations(mechanismId) {
  const defaults = {
    ts1: {
      O3: 8.1e-6,
      O2: 0.21,
      N2: 0.79,
    },
    chapman: {
      O2: 0.75,
      O: 0.0,
      O1D: 0.0,
      O3: 8.1e-6,
      M: 1.0,
    },
    analytical: {
      A: 1.0,
      B: 0.0,
      C: 0.0,
    },
  };

  return defaults[mechanismId] || {};
}

/**
 * Helper function to get default rate constants for a mechanism
 * @param {string} mechanismId - Mechanism ID
 * @returns {Object} Default rate constants (photolysis rates, etc.)
 */
export function getDefaultRateConstants(mechanismId) {
  const defaults = {
    chapman: {
      'PHOTO.jO2': 2.42e-17,
      'PHOTO.jO3->O': 1.15e-5,
      'PHOTO.jO3->O1D': 6.61e-9,
    },
  };

  return defaults[mechanismId] || {};
}

// Alias for backward compatibility
export const getDefaultPhotolysisRates = getDefaultRateConstants;

/**
 * Helper function to export simulation results as CSV
 * @param {Array} results - Simulation results
 * @param {string} filename - Output filename
 */
export function exportToCSV(results, filename = 'simulation_results.csv') {
  if (!results || results.length === 0) {
    console.error('No results to export');
    return;
  }

  // Get all species names from first result
  const firstResult = results[0];
  const species = Object.keys(firstResult).filter((key) => key !== 'time');

  // Create CSV header
  const header = ['time', ...species].join(',');

  // Create CSV rows
  const rows = results.map((result) => {
    const values = [result.time];
    species.forEach((sp) => {
      values.push(result[sp] || 0);
    });
    return values.join(',');
  });

  // Combine header and rows
  const csv = [header, ...rows].join('\n');

  // Create download link
  const blob = new Blob([csv], { type: 'text/csv' });
  const url = window.URL.createObjectURL(blob);
  const link = document.createElement('a');
  link.href = url;
  link.download = filename;
  link.click();
  window.URL.revokeObjectURL(url);
}

/**
 * Helper function to export simulation results as JSON
 * @param {Array} results - Simulation results
 * @param {string} filename - Output filename
 */
export function exportToJSON(results, filename = 'simulation_results.json') {
  if (!results || results.length === 0) {
    console.error('No results to export');
    return;
  }

  const json = JSON.stringify(results, null, 2);
  const blob = new Blob([json], { type: 'application/json' });
  const url = window.URL.createObjectURL(blob);
  const link = document.createElement('a');
  link.href = url;
  link.download = filename;
  link.click();
  window.URL.revokeObjectURL(url);
}

export default api;
