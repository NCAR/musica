import { useState, useMemo, useEffect } from 'react'
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, Label } from 'recharts'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from './ui/card'
import { Button } from './ui/button'
import { BarChart3, Atom, AlertCircle, Lightbulb } from 'lucide-react'

/**
 * SimulationChart Component
 * Displays atmospheric chemistry concentration data with interactive controls
 *
 * @param {Object} props
 * @param {Array} props.results - Simulation results array with time and concentrations
 * @param {Object} props.metadata - Simulation metadata (mechanism, duration, etc.)
 */
export function SimulationChart({ results, metadata }) {
  const [selectedSpecies, setSelectedSpecies] = useState([])
  const [showAll, setShowAll] = useState(true)
  const [initialized, setInitialized] = useState(false)

  // Generate color palette for species
  const colors = [
    '#3b82f6', '#ef4444', '#10b981', '#f59e0b',
    '#8b5cf6', '#ec4899', '#14b8a6', '#f97316',
    '#6366f1', '#84cc16', '#06b6d4', '#f43f5e',
    '#a855f7', '#22c55e', '#eab308', '#64748b'
  ]

  // Extract all species and filter significant ones
  const allSpecies = useMemo(() => {
    if (!results || results.length === 0) return []

    const MIN_VALUE = 1e-20
    const speciesNames = Object.keys(results[0].concentrations)

    // Filter species that have at least one significant value
    return speciesNames.filter(sp => {
      return results.some(result => result.concentrations[sp] > MIN_VALUE)
    })
  }, [results])

  // Reset initialization when results change
  useEffect(() => {
    setInitialized(false)
    setSelectedSpecies([])
  }, [results])

  // Initialize selected species on first render
  useEffect(() => {
    if (!initialized && allSpecies.length > 0 && selectedSpecies.length === 0) {
      // Calculate species metrics for selection
      const speciesMetrics = allSpecies.map(sp => {
        const values = results.map(r => r.concentrations[sp])
        const positiveValues = values.filter(v => v > 0)

        // Handle edge cases
        if (positiveValues.length === 0) {
          return { species: sp, range: 0, maxValue: 0, avgValue: 0 }
        }

        const max = Math.max(...values)
        const min = Math.min(...positiveValues)
        const avg = positiveValues.reduce((sum, v) => sum + v, 0) / positiveValues.length

        // Calculate variability range (max/min ratio)
        const range = (min > 0 && max > 0) ? max / min : max

        return {
          species: sp,
          range: isFinite(range) ? range : 0,
          maxValue: max,
          avgValue: avg
        }
      })

      console.log('Species metrics:', speciesMetrics.map(m => ({
        species: m.species,
        range: m.range.toFixed(3),
        maxValue: m.maxValue.toExponential(2)
      })))

      // First, try to select by variability (species with range > 1.0001, meaning they vary by at least 0.01%)
      // LOWERED from 1.001 to 1.0001 to catch even smaller changes
      let topSpecies = speciesMetrics
        .filter(m => m.range > 1.0001)
        .sort((a, b) => b.range - a.range)
        .slice(0, 8)
        .map(s => s.species)

      console.log('Variable species (range > 1.0001):', topSpecies)

      // Fallback 1: If we have fewer than 3 variable species, add species with highest concentrations
      if (topSpecies.length < 3) {
        console.log('Low variability detected, using concentration-based selection')
        const concentrationBased = speciesMetrics
          .sort((a, b) => b.maxValue - a.maxValue)
          .slice(0, 8)
          .map(s => s.species)

        // Combine variability-based and concentration-based, remove duplicates
        topSpecies = [...new Set([...topSpecies, ...concentrationBased])].slice(0, 8)
      }

      // Fallback 2: If still no species (edge case), just take first 8 species
      if (topSpecies.length === 0) {
        console.log('No species with significant values, selecting first available species')
        topSpecies = allSpecies.slice(0, 8)
      }

      console.log('Selected species for plot:', topSpecies)
      setSelectedSpecies(topSpecies)
      setInitialized(true)
    }
  }, [allSpecies, results, selectedSpecies.length, initialized])

  // Format data for chart
  const chartData = useMemo(() => {
    if (!results) return []

    const MIN_VALUE = 1e-20

    return results.map(result => {
      const point = {
        timeSeconds: result.time  // Use seconds instead of hours
      }

      allSpecies.forEach(species => {
        let value = result.concentrations[species]

        // CRITICAL FIX: MICM returns arrays (for multi-cell support), extract first element
        if (Array.isArray(value)) {
          value = value[0]
        }

        // For log scale, replace zeros with MIN_VALUE
        point[species] = value < MIN_VALUE ? MIN_VALUE : value
      })

      return point
    })
  }, [results, allSpecies])

  // Calculate Y-axis domain for log scale
  const yAxisDomain = useMemo(() => {
    if (!chartData.length) return [1e-20, 1]

    const displaySpecies = showAll ? allSpecies : selectedSpecies

    // Handle empty displaySpecies
    if (displaySpecies.length === 0) return [1e-20, 1]

    let minValue = Infinity
    let maxValue = -Infinity

    chartData.forEach(point => {
      displaySpecies.forEach(sp => {
        const value = point[sp]
        if (value > 1e-20) {
          minValue = Math.min(minValue, value)
          maxValue = Math.max(maxValue, value)
        }
      })
    })

    // Handle edge case where no valid values were found
    if (!isFinite(minValue) || !isFinite(maxValue)) {
      return [1e-20, 1]
    }

    // Ensure min < max with more padding for better visibility
    if (minValue >= maxValue) {
      return [minValue / 100, minValue * 100]
    }

    // Add extra padding for better line visibility (3 orders of magnitude instead of 1)
    const domain = [minValue / 1000, maxValue * 1000]
    console.log('Y-axis domain:', {
      minValue: minValue.toExponential(2),
      maxValue: maxValue.toExponential(2),
      domainMin: domain[0].toExponential(2),
      domainMax: domain[1].toExponential(2),
      displaySpecies: showAll ? 'all' : selectedSpecies.join(', ')
    })
    return domain
  }, [chartData, allSpecies, selectedSpecies, showAll])

  // Calculate X-axis domain dynamically
  const xAxisDomain = useMemo(() => {
    if (!chartData.length) return [0, 1]

    const times = chartData.map(point => point.timeSeconds).filter(t => isFinite(t))

    // Handle case where no valid times exist
    if (times.length === 0) return [0, 1]

    const minTime = Math.min(...times)
    const maxTime = Math.max(...times)

    // Validate the time values
    if (!isFinite(minTime) || !isFinite(maxTime)) {
      return [0, 1]
    }

    // Add padding if min and max are the same (single point or all points at same time)
    if (minTime === maxTime) {
      return [Math.max(0, minTime - 1), maxTime + 1]
    }

    return [minTime, maxTime]
  }, [chartData])

  // Toggle species selection
  const toggleSpecies = (species) => {
    setSelectedSpecies(prev =>
      prev.includes(species)
        ? prev.filter(s => s !== species)
        : [...prev, species]
    )
    setShowAll(false)
  }

  const displaySpecies = showAll ? allSpecies : selectedSpecies

  // Debug: log display species whenever it changes
  useEffect(() => {
    console.log('=== Chart Display Debug ===')
    console.log('Total species:', allSpecies.length, allSpecies)
    console.log('Selected species:', selectedSpecies.length, selectedSpecies)
    console.log('Show all?:', showAll)
    console.log('Actually displaying:', displaySpecies.length, displaySpecies)
    console.log('Data points:', chartData.length)
    if (chartData.length > 0 && displaySpecies.length > 0) {
      console.log('Sample data point 0:', chartData[0])
      console.log('Sample data point last:', chartData[chartData.length - 1])
    }
  }, [displaySpecies, allSpecies, selectedSpecies, showAll, chartData])

  // Validation checks
  if (!results || results.length === 0) {
    return (
      <Card>
        <CardContent className="flex items-center justify-center h-96">
          <div className="text-center text-gray-500">
            <div className="flex justify-center mb-2">
              <BarChart3 className="w-16 h-16" />
            </div>
            <p>No simulation data to display</p>
          </div>
        </CardContent>
      </Card>
    )
  }

  if (allSpecies.length === 0) {
    return (
      <Card>
        <CardContent className="flex items-center justify-center h-96">
          <div className="text-center text-gray-500">
            <div className="flex justify-center mb-2">
              <Atom className="w-16 h-16" />
            </div>
            <p>No species found in simulation results</p>
          </div>
        </CardContent>
      </Card>
    )
  }

  if (chartData.length === 0) {
    return (
      <Card>
        <CardContent className="flex items-center justify-center h-96">
          <div className="text-center text-gray-500">
            <div className="text-4xl mb-2">📉</div>
            <p>Unable to process chart data</p>
          </div>
        </CardContent>
      </Card>
    )
  }

  return (
    <Card>
      <CardHeader>
        <div>
          <CardTitle className="text-base xs:text-lg sm:text-xl">Concentration Profiles (Log Scale)</CardTitle>
          <CardDescription className="text-xs xs:text-sm">
            {metadata?.mechanism?.toUpperCase()} mechanism •
            {metadata?.duration?.toLocaleString()} seconds •
            {results.length} data points
          </CardDescription>
        </div>
      </CardHeader>

      <CardContent className="space-y-3 xs:space-y-4">
        {/* Warning for insufficient data points */}
        {results.length < 3 && (
          <div className="bg-yellow-50 border-2 border-yellow-300 rounded-lg p-3 text-sm">
            <p className="font-semibold text-yellow-800 mb-1 flex items-center gap-2">
              <AlertCircle className="w-4 h-4" />
              Limited Data Points
            </p>
            <p className="text-yellow-700 text-xs">
              This simulation produced only {results.length} data point{results.length > 1 ? 's' : ''}.
              For better visualization, consider increasing the simulation duration or decreasing the time step.
            </p>
          </div>
        )}

        {/* Species Filter */}
        <div className="border rounded-lg p-2 xs:p-3 sm:p-4 bg-gray-50">
          <div className="flex flex-col xs:flex-row items-start xs:items-center justify-between gap-2 xs:gap-0 mb-3">
            <h4 className="font-semibold text-xs xs:text-sm text-gray-900">Species Filter ({displaySpecies.length} selected)</h4>
            <div className="flex gap-2 w-full xs:w-auto">
              <Button
                variant="glass"
                size="sm"
                onClick={() => {
                  setShowAll(true)
                  setSelectedSpecies([])
                }}
                className="rounded-lg text-xs flex-1 xs:flex-initial"
              >
                Show All
              </Button>
              <Button
                variant="glass"
                size="sm"
                onClick={() => {
                  setShowAll(false)
                  setSelectedSpecies([])
                }}
                className="rounded-lg text-xs flex-1 xs:flex-initial"
              >
                Clear All
              </Button>
            </div>
          </div>

          <div className="flex flex-wrap gap-1.5 xs:gap-2 max-h-32 overflow-y-auto">
            {allSpecies.map((species, idx) => (
              <button
                key={species}
                onClick={() => toggleSpecies(species)}
                className={`px-2 xs:px-3 py-1 rounded-full text-xs font-medium transition-all ${
                  displaySpecies.includes(species)
                    ? 'bg-blue-500 text-white shadow-md'
                    : 'bg-gray-200 text-gray-600 hover:bg-gray-300'
                }`}
                style={
                  displaySpecies.includes(species)
                    ? { backgroundColor: colors[allSpecies.indexOf(species) % colors.length] }
                    : {}
                }
              >
                {species}
              </button>
            ))}
          </div>
        </div>

        {/* Chart */}
        <div className="border rounded-lg p-2 xs:p-3 sm:p-4 bg-white">
          <ResponsiveContainer width="100%" height={400} className="xs:hidden">
            <LineChart data={chartData} margin={{ top: 5, right: 5, left: 5, bottom: 5 }}>
              <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />

              <XAxis
                dataKey="timeSeconds"
                domain={['auto', 'auto']}
                stroke="#374151"
                tick={{ fontSize: 10, fill: '#374151' }}
                type="number"
              >
                <Label
                  value="Time (s)"
                  position="insideBottom"
                  offset={-5}
                  style={{ fill: '#1f2937', fontWeight: 600, fontSize: 11 }}
                />
              </XAxis>

              <YAxis
                scale="log"
                domain={[
                  (dataMin) => {
                    const min = dataMin > 0 ? dataMin / 10 : 1e-20
                    console.log('Y-axis dataMin:', dataMin, '→ domain min:', min)
                    return min
                  },
                  (dataMax) => {
                    const max = dataMax * 10
                    console.log('Y-axis dataMax:', dataMax, '→ domain max:', max)
                    return max
                  }
                ]}
                stroke="#374151"
                tick={{ fontSize: 8, fill: '#374151' }}
                tickFormatter={(value) => {
                  if (value === 0 || !isFinite(value)) return '0'
                  return value.toExponential(0)
                }}
                allowDataOverflow={false}
                width={38}
              />

              <Tooltip
                content={({ active, payload, label }) => {
                  if (!active || !payload?.length) return null

                  return (
                    <div className="bg-white border-2 border-gray-800 rounded-lg shadow-xl p-2" style={{ backgroundColor: 'white' }}>
                      <p className="font-semibold mb-1 text-xs text-gray-900" style={{ color: '#111827' }}>
                        Time: {label?.toLocaleString()} s
                      </p>
                      <div className="space-y-0.5">
                        {payload.map((entry, idx) => {
                          const numValue = typeof entry.value === 'number' ? entry.value : parseFloat(entry.value)
                          const isValidNumber = !isNaN(numValue) && isFinite(numValue)

                          return (
                            <div key={idx} className="flex items-center gap-1.5 text-xs" style={{ color: '#1f2937' }}>
                              <div
                                className="w-2 h-2 rounded-full flex-shrink-0"
                                style={{ backgroundColor: entry.color }}
                              />
                              <span className="font-medium text-gray-900" style={{ color: '#111827' }}>{entry.name}:</span>
                              <span className="font-mono text-gray-900 text-xs" style={{ color: '#111827' }}>
                                {isValidNumber
                                  ? (numValue < 1e-19 ? '0.00e+00' : numValue.toExponential(2))
                                  : 'N/A'}
                              </span>
                            </div>
                          )
                        })}
                      </div>
                    </div>
                  )
                }}
              />

              <Legend wrapperStyle={{ fontSize: '11px', fontWeight: '500', paddingTop: '10px' }} iconType="line" />

              {displaySpecies.map((species, idx) => (
                <Line
                  key={species}
                  type="monotone"
                  dataKey={species}
                  stroke={colors[allSpecies.indexOf(species) % colors.length]}
                  strokeWidth={2}
                  dot={results.length <= 10 ? { r: 3 } : false}
                  name={species}
                  connectNulls
                  isAnimationActive={false}
                />
              ))}
            </LineChart>
          </ResponsiveContainer>

          {/* Larger chart for bigger screens */}
          <ResponsiveContainer width="100%" height={600} className="hidden xs:block">
            <LineChart data={chartData} margin={{ top: 5, right: 30, left: 80, bottom: 5 }}>
              <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />

              <XAxis
                dataKey="timeSeconds"
                domain={['auto', 'auto']}
                stroke="#374151"
                tick={{ fontSize: 12, fill: '#374151' }}
                type="number"
              >
                <Label
                  value="Time (seconds)"
                  position="insideBottom"
                  offset={-5}
                  style={{ fill: '#1f2937', fontWeight: 600, fontSize: 14 }}
                />
              </XAxis>

              <YAxis
                scale="log"
                domain={[
                  (dataMin) => {
                    const min = dataMin > 0 ? dataMin / 10 : 1e-20
                    console.log('Y-axis dataMin:', dataMin, '→ domain min:', min)
                    return min
                  },
                  (dataMax) => {
                    const max = dataMax * 10
                    console.log('Y-axis dataMax:', dataMax, '→ domain max:', max)
                    return max
                  }
                ]}
                stroke="#374151"
                tick={{ fontSize: 11, fill: '#374151' }}
                tickFormatter={(value) => {
                  if (value === 0 || !isFinite(value)) return '0'
                  return value.toExponential(0)
                }}
                allowDataOverflow={false}
                width={70}
              >
                <Label
                  value="Concentration (mol mol⁻¹)"
                  angle={-90}
                  position="insideLeft"
                  offset={10}
                  style={{ fill: '#1f2937', fontWeight: 600, fontSize: 13, textAnchor: 'middle' }}
                />
              </YAxis>

              <Tooltip
                content={({ active, payload, label }) => {
                  if (!active || !payload?.length) return null

                  return (
                    <div className="bg-white border-2 border-gray-800 rounded-lg shadow-xl p-3" style={{ backgroundColor: 'white' }}>
                      <p className="font-semibold mb-2 text-sm text-gray-900" style={{ color: '#111827' }}>
                        Time: {label?.toLocaleString()} seconds
                      </p>
                      <div className="space-y-1">
                        {payload.map((entry, idx) => {
                          // Convert value to number and validate
                          const numValue = typeof entry.value === 'number' ? entry.value : parseFloat(entry.value)
                          const isValidNumber = !isNaN(numValue) && isFinite(numValue)

                          return (
                            <div key={idx} className="flex items-center gap-2 text-xs" style={{ color: '#1f2937' }}>
                              <div
                                className="w-3 h-3 rounded-full flex-shrink-0"
                                style={{ backgroundColor: entry.color }}
                              />
                              <span className="font-medium text-gray-900" style={{ color: '#111827' }}>{entry.name}:</span>
                              <span className="font-mono text-gray-900" style={{ color: '#111827' }}>
                                {isValidNumber
                                  ? (numValue < 1e-19 ? '0.0000e+00' : numValue.toExponential(4))
                                  : 'N/A'}
                              </span>
                            </div>
                          )
                        })}
                      </div>
                    </div>
                  )
                }}
              />

              <Legend
                wrapperStyle={{
                  fontSize: '13px',
                  fontWeight: '500',
                  paddingTop: '20px'
                }}
                iconType="line"
                content={({ payload }) => {
                  if (!payload || payload.length === 0) return null

                  return (
                    <div className="flex flex-wrap justify-center gap-3 px-4">
                      {payload.map((entry, index) => (
                        <div
                          key={`legend-${index}`}
                          className="flex items-center gap-2 px-3 py-1.5 bg-white border-2 rounded-lg shadow-sm"
                          style={{ borderColor: entry.color }}
                        >
                          <div
                            className="w-4 h-1 rounded"
                            style={{ backgroundColor: entry.color }}
                          />
                          <span className="text-sm font-semibold text-gray-900">
                            {entry.value}
                          </span>
                        </div>
                      ))}
                    </div>
                  )
                }}
              />

              {displaySpecies.map((species, idx) => (
                <Line
                  key={species}
                  type="monotone"
                  dataKey={species}
                  stroke={colors[allSpecies.indexOf(species) % colors.length]}
                  strokeWidth={3}
                  dot={results.length <= 10 ? { r: 4 } : false}
                  name={species}
                  connectNulls
                  isAnimationActive={false}
                />
              ))}
            </LineChart>
          </ResponsiveContainer>
        </div>

        {/* Info Box */}
        <div className="text-xs text-gray-600 bg-blue-50 border border-blue-200 rounded-lg p-3">
          <p className="font-semibold mb-1 flex items-center gap-2">
            <Lightbulb className="w-4 h-4" />
            Chart Controls:
          </p>
          <ul className="space-y-0.5 ml-4">
            <li>• Chart displays concentrations on <strong>logarithmic scale</strong> with dynamic axes</li>
            <li>• Click species badges to <strong>show/hide</strong> individual species</li>
            <li>• Use <strong>Show All</strong> to display all species simultaneously</li>
            <li>• Hover over the chart for detailed concentration values</li>
          </ul>
        </div>
      </CardContent>
    </Card>
  )
}

export default SimulationChart
