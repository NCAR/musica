// MusicBox API Service
// Handles all communication with the Express backend

import axios from 'axios';

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:3001/api';

// Create axios instance with default config
const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
  timeout: 60000, // 60 second timeout for long simulations
});

// API Service
export const musicBoxApi = {
  // Health check
  async checkHealth() {
    const response = await api.get('/health');
    return response.data;
  },

  // Mechanisms
  async getMechanisms() {
    const response = await api.get('/mechanisms');
    return response.data;
  },

  async getMechanism(mechanismId) {
    const response = await api.get(`/mechanisms/${mechanismId}`);
    return response.data;
  },

  async getMechanismSpecies(mechanismId) {
    const response = await api.get(`/mechanisms/${mechanismId}/species`);
    return response.data;
  },

  // Simulation
  async runSimulation(params) {
    const response = await api.post('/simulation/run', params);
    return response.data;
  },

  async getSimulation(simulationId) {
    const response = await api.get(`/simulation/${simulationId}`);
    return response.data;
  },

  async listSimulations() {
    const response = await api.get('/simulation');
    return response.data;
  },

  async deleteSimulation(simulationId) {
    const response = await api.delete(`/simulation/${simulationId}`);
    return response.data;
  },
};

// Error handling helper
export const handleApiError = (error) => {
  if (error.response) {
    // Server responded with error status
    return {
      message: error.response.data.message || error.response.data.error || 'Server error',
      status: error.response.status,
      data: error.response.data,
    };
  } else if (error.request) {
    // Request made but no response
    return {
      message: 'Unable to connect to server. Make sure the backend is running.',
      status: 0,
    };
  } else {
    // Something else happened
    return {
      message: error.message || 'Unknown error occurred',
      status: -1,
    };
  }
};

export default musicBoxApi;
