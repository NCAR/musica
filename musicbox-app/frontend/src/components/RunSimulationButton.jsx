import { useSelector, useDispatch } from 'react-redux'
import { useNavigate } from 'react-router-dom'
import { Button } from './ui/button'
import { runSimulation } from '../redux/slices/simulationSlice'
import { Loader2, Play } from 'lucide-react'

/**
 * RunSimulationButton Component
 * Compact button to run simulations from any page
 */
export function RunSimulationButton({ className = '' }) {
  const dispatch = useDispatch()
  const navigate = useNavigate()
  const simulation = useSelector((state) => state.simulation)
  const mechanism = useSelector((state) => state.mechanism.selectedMechanism)
  const mechanismData = useSelector((state) => state.mechanism)
  const currentExample = useSelector((state) => state.mechanism.currentExample)
  const conditions = useSelector((state) => state.conditions)

  const handleRunSimulation = async () => {
    // Prepare simulation configuration
    const config = {
      mechanism,
      species: mechanismData.species,
      reactions: mechanismData.reactions,
      temperature: conditions.initial.temperature,
      pressure: conditions.initial.pressure,
      timeStep: conditions.basic.timeStep,
      duration: conditions.basic.duration,
      initialConcentrations: conditions.initial.concentrations,
      rateConstants: conditions.rateConstants,
      outputFrequency: conditions.basic.outputFrequency,
    }

    try {
      // Run simulation
      await dispatch(runSimulation(config)).unwrap()

      // Navigate to results page on success (only if not already there)
      const currentPath = window.location.pathname
      if (!currentPath.includes('/plots')) {
        navigate('/plots')
      }
    } catch (error) {
      console.error('Simulation failed:', error)
      // Stay on current page to show error
    }
  }

  // Check if we have a valid mechanism configuration
  // For predefined mechanisms: need example loaded
  // For custom mechanisms: need at least 1 species and 1 reaction
  const isPredefinedMechanism = mechanism && mechanism !== 'custom'
  const isCustomMechanism = mechanism === 'custom'

  const hasValidPredefined = isPredefinedMechanism && currentExample && currentExample.id
  const hasValidCustom = isCustomMechanism && mechanismData.species.length > 0 && mechanismData.reactions.length > 0

  const hasValidMechanism = hasValidPredefined || hasValidCustom
  const isDisabled = simulation.status === 'running' || !hasValidMechanism

  // Generate helpful tooltip message
  const getTooltip = () => {
    if (simulation.status === 'running') return 'Simulation is currently running...'
    if (isCustomMechanism && mechanismData.species.length === 0) return 'Add at least 1 species to run simulation'
    if (isCustomMechanism && mechanismData.reactions.length === 0) return 'Add at least 1 reaction to run simulation'
    if (isPredefinedMechanism && !currentExample) return 'Please select an example mechanism'
    return 'Run atmospheric chemistry simulation'
  }

  return (
    <Button
      onClick={handleRunSimulation}
      disabled={isDisabled}
      variant="apple"
      size="lg"
      className={`rounded-2xl mt-2 mb-2 ${className}`}
      title={getTooltip()}
    >
      {simulation.status === 'running' ? (
        <>
          <Loader2 className="w-full h-4 mr-2 animate-spin" />
          Running...
        </>
      ) : (
        <>
          <Play className="w-full h-4 mr-2" />
          Run Simulation
        </>
      )}
    </Button>
  )
}

export default RunSimulationButton
