// Mechanism Redux Slice
// Manages chemical mechanism state (species, reactions, phases)
import { createSlice } from '@reduxjs/toolkit'

const initialState = {
  selectedMechanism: null,
  currentExample: null, // Track loaded example {id, name, description}
  species: [],
  reactions: [],
  phases: [],
  loading: false,
  error: null,
}

export const mechanismSlice = createSlice({
  name: 'mechanism',
  initialState,
  reducers: {
    setSelectedMechanism: (state, action) => {
      state.selectedMechanism = action.payload
    },
    setCurrentExample: (state, action) => {
      state.currentExample = action.payload
    },
    setSpecies: (state, action) => {
      state.species = action.payload
    },
    addSpecies: (state, action) => {
      state.species.push(action.payload)
    },
    updateSpecies: (state, action) => {
      const index = state.species.findIndex(s => s.name === action.payload.name)
      if (index !== -1) {
        state.species[index] = action.payload
      }
    },
    removeSpecies: (state, action) => {
      state.species = state.species.filter(s => s.name !== action.payload)
    },
    setReactions: (state, action) => {
      state.reactions = action.payload
    },
    addReaction: (state, action) => {
      state.reactions.push(action.payload)
    },
    updateReaction: (state, action) => {
      const index = state.reactions.findIndex(r => r.id === action.payload.id)
      if (index !== -1) {
        state.reactions[index] = action.payload
      }
    },
    removeReaction: (state, action) => {
      state.reactions = state.reactions.filter(r => r.id !== action.payload)
    },
    setLoading: (state, action) => {
      state.loading = action.payload
    },
    setError: (state, action) => {
      state.error = action.payload
    },
    resetMechanism: () => initialState,
  },
})

export const {
  setSelectedMechanism,
  setCurrentExample,
  setSpecies,
  addSpecies,
  updateSpecies,
  removeSpecies,
  setReactions,
  addReaction,
  updateReaction,
  removeReaction,
  setLoading,
  setError,
  resetMechanism,
} = mechanismSlice.actions

export default mechanismSlice.reducer
