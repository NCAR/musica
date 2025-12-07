// Simulation Redux Slice
// Manages simulation execution and results
import { createSlice, createAsyncThunk } from '@reduxjs/toolkit'
import axios from 'axios'

const API_URL = 'http://localhost:3001/api'

// Async thunk for running simulation
export const runSimulation = createAsyncThunk(
  'simulation/run',
  async (config, { rejectWithValue }) => {
    try {
      const response = await axios.post(`${API_URL}/simulation/run`, config)
      return response.data
    } catch (error) {
      return rejectWithValue(error.response?.data || { error: error.message })
    }
  }
)

// Async thunk for polling simulation status (for future long-running sims)
export const checkSimulationStatus = createAsyncThunk(
  'simulation/checkStatus',
  async (simulationId, { rejectWithValue }) => {
    try {
      const response = await axios.get(`${API_URL}/simulation/${simulationId}`)
      return response.data
    } catch (error) {
      return rejectWithValue(error.response?.data || { error: error.message })
    }
  }
)

const initialState = {
  status: 'idle', // 'idle' | 'loading' | 'running' | 'succeeded' | 'failed'
  results: null,
  environmentalData: null,
  simulationId: null,
  error: null,
  metadata: null,
  // For polling
  pollingInterval: null,
}

export const simulationSlice = createSlice({
  name: 'simulation',
  initialState,
  reducers: {
    setStatus: (state, action) => {
      state.status = action.payload
    },
    setResults: (state, action) => {
      state.results = action.payload
    },
    setError: (state, action) => {
      state.error = action.payload
    },
    setSimulationId: (state, action) => {
      state.simulationId = action.payload
    },
    setPollingInterval: (state, action) => {
      state.pollingInterval = action.payload
    },
    clearSimulation: (state) => {
      state.status = 'idle'
      state.results = null
      state.environmentalData = null
      state.simulationId = null
      state.error = null
      state.metadata = null
    },
    resetSimulation: () => initialState,
  },
  extraReducers: (builder) => {
    builder
      // Run simulation
      .addCase(runSimulation.pending, (state) => {
        state.status = 'running'
        state.error = null
      })
      .addCase(runSimulation.fulfilled, (state, action) => {
        state.status = 'succeeded'
        state.results = action.payload.results
        state.environmentalData = action.payload.environmentalData || null
        state.simulationId = action.payload.simulationId
        state.metadata = action.payload.metadata
      })
      .addCase(runSimulation.rejected, (state, action) => {
        state.status = 'failed'
        state.error = action.payload
      })
      // Check status (for polling)
      .addCase(checkSimulationStatus.fulfilled, (state, action) => {
        const { simulation } = action.payload
        if (simulation.status === 'completed') {
          state.status = 'succeeded'
          state.results = simulation.results
          state.metadata = simulation.parameters
        } else if (simulation.status === 'running') {
          state.status = 'running'
        } else if (simulation.status === 'failed') {
          state.status = 'failed'
          state.error = simulation.error
        }
      })
  },
})

export const {
  setStatus,
  setResults,
  setError,
  setSimulationId,
  setPollingInterval,
  clearSimulation,
  resetSimulation,
} = simulationSlice.actions

export default simulationSlice.reducer
