import { useSelector, useDispatch } from 'react-redux'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../ui/card'
import { setDuration, setTimeStep, setOutputFrequency } from '../../redux/slices/conditionsSlice'
import { Info } from 'lucide-react'

/**
 * BasicConfigTab Component
 * Manages basic simulation configuration (duration, timestep, output frequency)
 */
export function BasicConfigTab() {
  const dispatch = useDispatch()
  const basic = useSelector((state) => state.conditions.basic)

  const handleDurationChange = (e) => {
    const hours = parseFloat(e.target.value)
    if (!isNaN(hours)) {
      dispatch(setDuration(hours * 3600)) // Convert hours to seconds
    }
  }

  const handleTimeStepChange = (e) => {
    const value = parseFloat(e.target.value)
    if (!isNaN(value)) {
      dispatch(setTimeStep(value))
    }
  }

  const handleOutputFrequencyChange = (e) => {
    const value = parseInt(e.target.value)
    if (!isNaN(value)) {
      dispatch(setOutputFrequency(value))
    }
  }

  return (
    <div className="space-y-4">
      <Card>
        <CardHeader>
          <CardTitle>Simulation Duration</CardTitle>
          <CardDescription>Configure how long the simulation runs</CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
          <div>
            <label className="block text-sm font-semibold text-blue-100 mb-2">
              Duration (hours)
            </label>
            <input
              type="number"
              value={basic.duration / 3600}
              onChange={handleDurationChange}
              step="0.1"
              min="0"
              className="w-full px-3 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent"
            />
            <p className="text-xs text-gray-500 mt-1">
              Total simulation time: {basic.duration} seconds
            </p>
          </div>
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle>Time Step Configuration</CardTitle>
          <CardDescription>
            Control the temporal resolution of the simulation
          </CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
          <div>
            <label className="block text-sm font-semibold text-blue-100 mb-2">
              Time Step (seconds)
            </label>
            <input
              type="number"
              value={basic.timeStep}
              onChange={handleTimeStepChange}
              step="10"
              min="1"
              className="w-full px-3 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent"
            />
            <p className="text-xs text-gray-500 mt-1">
              Smaller timesteps = more accurate but slower
            </p>
          </div>

          <div>
            <label className="block text-sm font-semibold text-blue-100 mb-2">
              Output Frequency
            </label>
            <input
              type="number"
              value={basic.outputFrequency}
              onChange={handleOutputFrequencyChange}
              step="1"
              min="1"
              className="w-full px-3 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent"
            />
            <p className="text-xs text-gray-500 mt-1">
              Save output every {basic.outputFrequency} timesteps
            </p>
          </div>
        </CardContent>
      </Card>

      <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-3 text-xs text-gray-300">
        <p className="font-semibold mb-1 flex items-center gap-2">
          <Info className="w-4 h-4" />
          Configuration Summary:
        </p>
        <ul className="space-y-0.5 ml-4">
          <li>• Total steps: {Math.floor(basic.duration / basic.timeStep)}</li>
          <li>• Output points: ~{Math.floor(basic.duration / basic.timeStep / basic.outputFrequency)}</li>
          <li>• Simulation end time: {(basic.duration / 3600).toFixed(2)} hours</li>
        </ul>
      </div>
    </div>
  )
}

export default BasicConfigTab
