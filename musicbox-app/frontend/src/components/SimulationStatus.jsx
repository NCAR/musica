import { useEffect, useRef, useState } from 'react'
import { useSelector, useDispatch } from 'react-redux'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from './ui/card'
import { Button } from './ui/button'
import { runSimulation, setStatus } from '../redux/slices/simulationSlice'
import { useNavigate } from 'react-router-dom'
import { useToast } from '@/hooks/use-toast'
import { Pause, AlertCircle, Zap, CheckCircle2, XCircle, HelpCircle, Loader2, Play, BarChart3, Lightbulb } from 'lucide-react'

/**
 * SimulationStatus Component
 * Displays simulation status with polling support for long-running simulations
 */
export function SimulationStatus() {
  const dispatch = useDispatch()
  const navigate = useNavigate()
  const { toast } = useToast()
  const simulation = useSelector((state) => state.simulation)
  const mechanism = useSelector((state) => state.mechanism.selectedMechanism)
  const currentExample = useSelector((state) => state.mechanism.currentExample)
  const conditions = useSelector((state) => state.conditions)

  const [isSubmitting, setIsSubmitting] = useState(false)
  const renderCount = useRef(0)
  const pollingIntervalRef = useRef(null)

  useEffect(() => {
    renderCount.current++
  })

  // Cleanup polling on unmount
  useEffect(() => {
    return () => {
      if (pollingIntervalRef.current) {
        clearInterval(pollingIntervalRef.current)
      }
    }
  }, [])

  const handleRunSimulation = async () => {
    // Validate that an example is selected
    if (!mechanism || !currentExample) {
      toast({
        variant: 'destructive',
        title: 'No Example Selected',
        description: 'Please select an example from the list before running the simulation.',
      })
      return
    }

    // Prevent spam clicking
    if (isSubmitting) {
      return
    }

    setIsSubmitting(true)

    // Clear any existing polling
    if (pollingIntervalRef.current) {
      clearInterval(pollingIntervalRef.current)
    }

    // Prepare simulation configuration
    const config = {
      mechanism,
      temperature: conditions.initial.temperature,
      pressure: conditions.initial.pressure,
      timeStep: conditions.basic.timeStep,
      duration: conditions.basic.duration,
      initialConcentrations: conditions.initial.concentrations,
      rateConstants: conditions.rateConstants,
      outputFrequency: conditions.basic.outputFrequency,
    }

    // Add evolving conditions if enabled
    if (conditions.evolving && conditions.evolving.enabled && conditions.evolving.times.length > 0) {
      config.evolvingConditions = {
        enabled: true,
        times: conditions.evolving.times,
        temperature: conditions.evolving.temperature,
        pressure: conditions.evolving.pressure,
        interpolationMethod: conditions.evolving.interpolationMethod || 'linear',
      }
    }

    try {
      // Run simulation (returns immediately for our current sync implementation)
      await dispatch(runSimulation(config)).unwrap()
    } catch (error) {
      console.error('Simulation failed:', error)
    } finally {
      // Re-enable button after a short delay
      setTimeout(() => {
        setIsSubmitting(false)
      }, 1000)
    }
  }

  const getStatusDisplay = () => {
    switch (simulation.status) {
      case 'idle':
        // Check if example is selected
        const hasExample = currentExample && currentExample.id
        return {
          color: hasExample ? 'green' : 'orange',
          Icon: hasExample ? Pause : AlertCircle,
          title: hasExample ? 'Ready to Run' : 'Please choose examples to start your simulation',
          message: hasExample
            ? 'Click the Run button to start your simulation'
            : 'Select an example from the Dashboard to begin',
        }
      case 'running':
        return {
          color: 'blue',
          Icon: Zap,
          title: 'Running Simulation',
          message: 'Your simulation is currently running...',
        }
      case 'succeeded':
        return {
          color: 'green',
          Icon: CheckCircle2,
          title: 'Simulation Complete!',
          message: 'Your simulation finished successfully',
        }
      case 'failed':
        return {
          color: 'red',
          Icon: XCircle,
          title: 'Simulation Failed',
          message: simulation.error?.message || 'An error occurred',
        }
      default:
        return {
          color: 'gray',
          Icon: HelpCircle,
          title: 'Unknown Status',
          message: 'Unexpected simulation state',
        }
    }
  }

  const status = getStatusDisplay()

  return (
    <Card>
      <CardHeader>
        <div className="flex items-center justify-between">
          <div>
            <CardTitle>Simulation Status</CardTitle>
            <CardDescription className="text-white italic">Monitor your simulation execution</CardDescription>
          </div>

          {/* <Button
            onClick={handleRunSimulation}
            disabled={simulation.status === 'running' || !mechanism || !currentExample || isSubmitting}
            variant="apple"
            size="default"
            className="rounded-2xl"
            title={!mechanism || !currentExample ? 'Please select an example to run simulation' : ''}
          >
            {simulation.status === 'running' ? (
              <>
                <Loader2 className="w-4 h-4 mr-2 animate-spin" />
                Running...
              </>
            ) : (
              <>
                <Play className="w-4 h-4 mr-2" />
                Run Simulation
              </>
            )}
          </Button> */}
        </div>
      </CardHeader>

      <CardContent>
        <div
          className={`p-6 rounded-lg border-2 bg-white/0 backdrop-blur-lg ${
            status.color === 'green'
              ? 'border-green-400/50'
              : status.color === 'blue'
              ? 'border-blue-400/50'
              : status.color === 'red'
              ? 'border-red-400/50'
              : status.color === 'orange'
              ? 'border-orange-400/50'
              : 'border-white/20'
          }`}
        >
          <div className="flex items-center gap-3 mb-3">
            <status.Icon className={`w-10 h-10 ${
              status.color === 'green'
                ? 'text-green-400'
                : status.color === 'blue'
                ? 'text-blue-400'
                : status.color === 'red'
                ? 'text-red-400'
                : status.color === 'orange'
                ? 'text-orange-400'
                : 'text-gray-400'
            }`} />
            <div>
              <h3 className="font-semibold text-lg text-white">{status.title}</h3>
              <p className="text-sm text-gray-300">{status.message}</p>
            </div>
          </div>

          {simulation.status === 'running' && (
            <div className="mt-4">
              <div className="flex items-center gap-2">
                <div className="animate-spin h-5 w-5 border-2 border-blue-400 border-t-transparent rounded-full"></div>
                <span className="text-sm text-gray-300">
                  Processing... (render #{renderCount.current})
                </span>
              </div>
            </div>
          )}

          {simulation.status === 'succeeded' && simulation.metadata && (
            <div className="mt-4 space-y-2">
              <p className="text-sm text-white-300">
                <strong className="text-white font-semibold">Mechanism:</strong> {simulation.metadata.mechanism?.toUpperCase()}
              </p>
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Duration:</strong> {simulation.metadata.duration}s
              </p>
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Output Points:</strong> {simulation.metadata.outputPoints}
              </p>

              <div className="flex gap-2 mt-4">
                <Button
                  variant="glass"
                  size="default"
                  onClick={() => navigate('/plots')}
                  className="rounded-2xl"
                >
                  <BarChart3 className="w-4 h-4 mr-2" />
                  View Results
                </Button>
              </div>
            </div>
          )}

          {simulation.status === 'failed' && simulation.error && (
            <div className="mt-4 p-3 bg-red-900/20 backdrop-blur-lg border border-red-400/30 rounded text-sm">
              <strong className="text-red-400">Error Details:</strong>
              <pre className="mt-2 text-xs overflow-auto text-gray-300">
                {JSON.stringify(simulation.error, null, 2)}
              </pre>
            </div>
          )}

          {/* Show loaded example info when in idle state */}
          {simulation.status === 'idle' && currentExample && currentExample.id && (
            <div className="mt-4 space-y-2">
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Example:</strong> {currentExample.name}
              </p>
              {currentExample.description && (
                <p className="text-xs text-gray-400 italic">
                  {currentExample.description}
                </p>
              )}
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Mechanism:</strong> {mechanism?.toUpperCase()}
              </p>
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Duration:</strong> {conditions.basic.duration}s
              </p>
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Temperature:</strong> {conditions.initial.temperature}K
              </p>
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Pressure:</strong> {conditions.initial.pressure}Pa
              </p>
              <p className="text-sm text-gray-300">
                <strong className="text-white font-semibold">Species:</strong> {Object.keys(conditions.initial.concentrations || {}).length}
              </p>
              {conditions.rateConstants && Object.keys(conditions.rateConstants).length > 0 && (
                <p className="text-sm text-gray-300">
                  <strong className="text-white font-semibold">Rate Constants:</strong> {Object.keys(conditions.rateConstants).length}
                </p>
              )}
            </div>
          )}
        </div>

        {simulation.status === 'idle' && !currentExample && (
          <div className="mt-4 bg-white/0 backdrop-blur-lg border border-white/20 rounded-lg p-3 text-xs text-white-300">
            <p className="font-semibold mb-1 flex items-center gap-2">
              <Lightbulb className="w-4 h-4" />
              Before running:
            </p>
            <ul className="space-y-0.5 ml-4">
              <li>• Select an example from the Dashboard</li>
              <li>• Or configure your own mechanism in the Mechanism tab</li>
              <li>• Set initial conditions in the Conditions tab</li>
              <li>• Adjust simulation duration and timestep as needed</li>
            </ul>
          </div>
        )}
      </CardContent>
    </Card>
  )
}

export default SimulationStatus
