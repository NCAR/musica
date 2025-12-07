import { useState, useEffect } from 'react'
import { useDispatch } from 'react-redux'
import axios from 'axios'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from './ui/card'
import { Button } from './ui/button'
import { loadConditions } from '../redux/slices/conditionsSlice'
import { setSelectedMechanism, setCurrentExample } from '../redux/slices/mechanismSlice'
import { clearSimulation } from '../redux/slices/simulationSlice'

const API_URL = 'http://localhost:3001/api'

/**
 * ExampleLoader Component
 * Loads pre-configured example simulations
 */
export function ExampleLoader({ onExampleSelected }) {
  const [examples, setExamples] = useState([])
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState(null)
  const dispatch = useDispatch()

  // Fetch available examples on component mount
  useEffect(() => {
    fetchExamples()
  }, [])

  const fetchExamples = async () => {
    try {
      const response = await axios.get(`${API_URL}/examples`)
      setExamples(response.data.examples)
    } catch (err) {
      console.error('Error fetching examples:', err)
      setError('Failed to load examples')
    }
  }

  const loadExample = async (exampleId) => {
    setLoading(true)
    setError(null)

    try {
      const response = await axios.get(`${API_URL}/examples/${exampleId}`)
      const exampleData = response.data.example

      // Find the example metadata from the list
      const exampleMetadata = examples.find(ex => ex.id === exampleId)

      // CRITICAL: Clear old simulation results before loading new example
      // This ensures Simulation Status shows the new example info, not old results
      dispatch(clearSimulation())

      // Update Redux store with example configuration
      dispatch(setSelectedMechanism(exampleData.mechanism))
      dispatch(loadConditions(exampleData.conditions))
      dispatch(setCurrentExample({
        id: exampleId,
        name: exampleData.name,
        description: exampleData.description || exampleMetadata?.description,
      }))

      console.log(`Loaded example: ${exampleData.name}`)

      // Call callback to notify parent that example was selected
      if (onExampleSelected) {
        onExampleSelected()
      }
    } catch (err) {
      console.error('Error loading example:', err)
      setError(`Failed to load example: ${exampleId}`)
    } finally {
      setLoading(false)
    }
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>Example Simulations</CardTitle>
        <CardDescription>
          Load pre-configured example simulations to get started quickly
        </CardDescription>
      </CardHeader>

      <CardContent>
        {error && (
          <div className="bg-red-900/20 backdrop-blur-lg border border-red-400/30 text-red-400 px-4 py-3 rounded mb-4">
            {error}
          </div>
        )}

        <div className="grid gap-3">
          {examples.map((example) => (
            <div
              key={example.id}
              className="flex items-center justify-between p-4 border border-white/20 rounded-lg bg-white/5 hover:bg-white/10 transition-colors"
            >
              <div className="flex-1">
                <h4 className="font-semibold text-sm">{example.name}</h4>
                <p className="text-xs text-gray-100 mt-1">{example.description}</p>
                <span className="inline-block mt-2 px-2 py-1 bg-white/5 backdrop-blur-lg border border-white/20 text-white text-xs font-bold rounded">
                  {example.mechanism.toUpperCase()}
                </span>
              </div>

              <Button
                variant="glass"
                size="sm"
                onClick={() => loadExample(example.id)}
                disabled={loading}
                className="rounded-2xl ml-4"
              >
                {loading ? 'Loading...' : 'Load'}
              </Button>
            </div>
          ))}
        </div>

        {examples.length === 0 && !error && (
          <p className="text-center text-gray-500 py-4">No examples available</p>
        )}
      </CardContent>
    </Card>
  )
}

export default ExampleLoader
