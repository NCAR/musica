import { useSelector } from 'react-redux'
import SimulationChart from '../SimulationChart'
import { Card, CardContent } from '../ui/card'
import ErrorBoundary from '../ErrorBoundary'
import { BarChart3 } from 'lucide-react'

/**
 * SpeciesPlot Component
 * Displays chemical species concentrations over time
 */
export function SpeciesPlot() {
  const simulation = useSelector((state) => state.simulation)

  if (!simulation.results || simulation.status !== 'succeeded') {
    return (
      <Card>
        <CardContent className="flex items-center justify-center h-96">
          <div className="text-center text-white-500">
            <div className="flex justify-center mb-2">
              <BarChart3 className="w-12 h-12" />
            </div>
            <p>Run a simulation to see species concentration plots</p>
          </div>
        </CardContent>
      </Card>
    )
  }

  return (
    <ErrorBoundary>
      <SimulationChart
        results={simulation.results}
        metadata={simulation.metadata}
      />
    </ErrorBoundary>
  )
}

export default SpeciesPlot
