// Redux Store Configuration
import { configureStore } from '@reduxjs/toolkit'
import mechanismReducer from '../slices/mechanismSlice'
import conditionsReducer from '../slices/conditionsSlice'
import simulationReducer from '../slices/simulationSlice'

export const store = configureStore({
  reducer: {
    mechanism: mechanismReducer,
    conditions: conditionsReducer,
    simulation: simulationReducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      serializableCheck: {
        // Ignore these action types
        ignoredActions: ['simulation/setResults'],
        // Ignore these paths in the state
        ignoredPaths: ['simulation.results'],
      },
    }),
})
