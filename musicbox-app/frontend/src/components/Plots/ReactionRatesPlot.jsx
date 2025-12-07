import { useState, useMemo, useEffect } from 'react'
import { useSelector } from 'react-redux'
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, Label } from 'recharts'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../ui/card'
import { Button } from '../ui/button'
import { FlaskConical, AlertCircle, Lightbulb } from 'lucide-react'

/**
 * ReactionRatesPlot Component
 * Displays photolysis and user-defined reaction rate parameters over time
 */
export function ReactionRatesPlot() {
  const simulation = useSelector((state) => state.simulation)
  const [selectedRates, setSelectedRates] = useState([])
  const [showAll, setShowAll] = useState(true)
  const [initialized, setInitialized] = useState(false)

  // Generate color palette for reaction rates
  const colors = [
    '#3b82f6', '#ef4444', '#10b981', '#f59e0b',
    '#8b5cf6', '#ec4899', '#14b8a6', '#f97316',
    '#6366f1', '#84cc16', '#06b6d4', '#f43f5e'
  ]

  // Extract rate constants from initial concentrations (they're set via setUserDefinedRateParameters)
  // In MICM, photolysis rates are prefixed with "PHOTO." and user-defined with "USER."
  const rateParameters = useMemo(() => {
    if (!simulation.metadata?.rateConstants) {
      console.log('ReactionRatesPlot: No rateConstants in metadata')
      return []
    }

    const allKeys = Object.keys(simulation.metadata.rateConstants)
    console.log('ReactionRatesPlot: All rate constant keys:', allKeys)

    // Filter for PHOTO. and USER. prefixed parameters
    const filtered = allKeys.filter(key => key.startsWith('PHOTO.') || key.startsWith('USER.'))
    console.log('ReactionRatesPlot: Filtered rate parameters:', filtered)

    // Log the actual values
    const rateValues = {}
    filtered.forEach(key => {
      rateValues[key] = simulation.metadata.rateConstants[key]
    })
    console.log('ReactionRatesPlot: Rate parameter values:', rateValues)

    return filtered.sort()
  }, [simulation.metadata])

  // Initialize selected rates
  useEffect(() => {
    if (!initialized && rateParameters.length > 0 && selectedRates.length === 0) {
      const initialSelection = rateParameters.slice(0, Math.min(8, rateParameters.length))
      setSelectedRates(initialSelection)
      setInitialized(true)
    }
  }, [rateParameters, selectedRates.length, initialized])

  // Format data for chart (rates are constant over time for now)
  const chartData = useMemo(() => {
    if (!simulation.results || !simulation.metadata?.rateConstants) return []

    return simulation.results.map(result => {
      const point = {
        timeSeconds: result.time
      }

      rateParameters.forEach(param => {
        point[param] = simulation.metadata.rateConstants[param]
      })

      return point
    })
  }, [simulation.results, simulation.metadata, rateParameters])

  // Toggle rate selection
  const toggleRate = (rate) => {
    setSelectedRates(prev =>
      prev.includes(rate)
        ? prev.filter(r => r !== rate)
        : [...prev, rate]
    )
    setShowAll(false)
  }

  // Determine which rates to display
  const displayRates = showAll ? rateParameters : selectedRates

  // Determine scale type and calculate Y-axis domain
  const { scaleType, yDomain } = useMemo(() => {
    if (!simulation.metadata?.rateConstants || rateParameters.length === 0) {
      return { scaleType: 'log', yDomain: ['auto', 'auto'] }
    }

    // Get all rate values for displayed rates
    const values = displayRates
      .map(param => simulation.metadata.rateConstants[param])
      .filter(val => val !== undefined && val !== null && isFinite(val))

    console.log('ReactionRatesPlot: Displayed rate values:', values)

    if (values.length === 0) {
      console.warn('ReactionRatesPlot: No valid rate values found!')
      return { scaleType: 'linear', yDomain: [0, 1] }
    }

    const minValue = Math.min(...values)
    const maxValue = Math.max(...values)

    console.log(`ReactionRatesPlot: Value range: ${minValue} to ${maxValue}`)

    // Use linear scale if:
    // 1. Any value is zero or negative
    // 2. All values are very small (< 1e-10)
    // 3. Range is too small for log scale
    if (minValue <= 0) {
      console.log('ReactionRatesPlot: Using LINEAR scale (values <= 0)')
      return {
        scaleType: 'linear',
        yDomain: [Math.min(0, minValue * 1.1), maxValue * 1.1]
      }
    }

    if (maxValue < 1e-10) {
      console.log('ReactionRatesPlot: Using LINEAR scale (all values < 1e-10)')
      return {
        scaleType: 'linear',
        yDomain: [0, maxValue * 1.2]
      }
    }

    // Use log scale for positive values with sufficient range
    const range = maxValue / minValue
    if (range < 10) {
      console.log('ReactionRatesPlot: Using LINEAR scale (range too small)')
      return {
        scaleType: 'linear',
        yDomain: [minValue * 0.9, maxValue * 1.1]
      }
    }

    console.log('ReactionRatesPlot: Using LOG scale')
    // For log scale, use values that are powers of 10
    const logMin = Math.floor(Math.log10(minValue))
    const logMax = Math.ceil(Math.log10(maxValue))
    return {
      scaleType: 'log',
      yDomain: [Math.pow(10, logMin), Math.pow(10, logMax)]
    }
  }, [simulation.metadata, rateParameters, displayRates])

  if (!simulation.results || simulation.status !== 'succeeded') {
    return (
      <Card>
        <CardHeader>
          <CardTitle>Reaction Rates</CardTitle>
          <CardDescription className="text-white-500 italic">
            View photolysis and user-defined reaction rate parameters
          </CardDescription>
        </CardHeader>

        <CardContent>
          <div className="text-center py-12 text-white-500">
            <div className="flex justify-center mb-4">
              <FlaskConical className="w-16 h-16" />
            </div>
            <p>Run a simulation to see reaction rate parameters</p>
          </div>
        </CardContent>
      </Card>
    )
  }

  if (rateParameters.length === 0) {
    return (
      <Card>
        <CardHeader>
          <CardTitle>Reaction Rates</CardTitle>
          <CardDescription>
            No rate parameters found in this simulation
          </CardDescription>
        </CardHeader>

        <CardContent>
          <div className="text-center py-12 text-gray-500">
            <div className="flex justify-center mb-4">
              <AlertCircle className="w-16 h-16 text-yellow-500" />
            </div>
            <p className="text-sm max-w-md mx-auto mb-3">
              This mechanism does not have photolysis or user-defined reactions.
            </p>
            <div className="text-xs bg-yellow-50 border border-yellow-200 rounded-lg p-3 max-w-md mx-auto">
              <p className="font-semibold mb-1">Troubleshooting:</p>
              <ul className="text-left space-y-1">
                <li>• Check if your mechanism config includes PHOTOLYSIS or USER_DEFINED reactions</li>
                <li>• Verify rate constants are set with PHOTO.* or USER.* prefixes</li>
                <li>• See console for detailed debugging information</li>
              </ul>
            </div>
          </div>
        </CardContent>
      </Card>
    )
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle>Reaction Rate Parameters</CardTitle>
        <CardDescription>
          {simulation.metadata?.mechanism?.toUpperCase()} mechanism •
          {rateParameters.length} rate parameter{rateParameters.length !== 1 ? 's' : ''}
        </CardDescription>
      </CardHeader>

      <CardContent className="space-y-4">
        {/* Rate Parameter Filter */}
        <div className="border rounded-lg p-4 bg-gray-50">
          <div className="flex items-center justify-between mb-3">
            <h4 className="font-semibold text-sm text-gray-900">
              Rate Parameters ({displayRates.length} selected)
            </h4>
            <div className="flex gap-2">
              <Button
                variant="glass"
                size="sm"
                onClick={() => {
                  setShowAll(true)
                  setSelectedRates([])
                }}
                className="rounded-lg text-xs"
              >
                Show All
              </Button>
              <Button
                variant="glass"
                size="sm"
                onClick={() => {
                  setShowAll(false)
                  setSelectedRates([])
                }}
                className="rounded-lg text-xs"
              >
                Clear All
              </Button>
            </div>
          </div>

          <div className="flex flex-wrap gap-2 max-h-32 overflow-y-auto">
            {rateParameters.map((rate, idx) => {
              const isPhotolysis = rate.startsWith('PHOTO.')
              return (
                <button
                  key={rate}
                  onClick={() => toggleRate(rate)}
                  className={`px-3 py-1 rounded-full text-xs font-medium transition-all ${
                    displayRates.includes(rate)
                      ? 'text-white shadow-md'
                      : 'bg-gray-200 text-gray-600 hover:bg-gray-300'
                  }`}
                  style={
                    displayRates.includes(rate)
                      ? { backgroundColor: colors[idx % colors.length] }
                      : {}
                  }
                  title={isPhotolysis ? 'Photolysis reaction' : 'User-defined reaction'}
                >
                  {rate}
                </button>
              )
            })}
          </div>
        </div>

        {/* Chart */}
        <div className="border rounded-lg p-4 bg-white">
          {chartData.length === 0 ? (
            <div className="text-center py-12 text-gray-500">
              <AlertCircle className="w-12 h-12 mx-auto mb-3 text-yellow-500" />
              <p className="text-sm">No chart data available</p>
              <p className="text-xs mt-2">Check console for debugging information</p>
            </div>
          ) : (
            <ResponsiveContainer width="100%" height={500}>
              <LineChart data={chartData} margin={{ top: 5, right: 30, left: 100, bottom: 5 }}>
              <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />

              <XAxis
                dataKey="timeSeconds"
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
                scale={scaleType}
                domain={yDomain}
                allowDataOverflow={false}
                stroke="#374151"
                tick={{ fontSize: 11, fill: '#374151' }}
                tickFormatter={(value) => {
                  if (value === 0 || !isFinite(value)) return '0'
                  if (scaleType === 'log' || Math.abs(value) >= 1000 || Math.abs(value) < 0.01) {
                    return value.toExponential(0)
                  }
                  return value.toFixed(2)
                }}
                width={90}
              >
                <Label
                  value="Rate Constant (s⁻¹)"
                  angle={-90}
                  position="insideLeft"
                  offset={15}
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
                        {payload.map((entry, idx) => (
                          <div key={idx} className="flex items-center gap-2 text-xs" style={{ color: '#1f2937' }}>
                            <div
                              className="w-3 h-3 rounded-full flex-shrink-0"
                              style={{ backgroundColor: entry.color }}
                            />
                            <span className="font-medium text-gray-900" style={{ color: '#111827' }}>{entry.name}:</span>
                            <span className="font-mono text-gray-900" style={{ color: '#111827' }}>
                              {entry.value?.toExponential(4) || 'N/A'}
                            </span>
                          </div>
                        ))}
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

              {displayRates.map((rate, idx) => {
                const rateValue = simulation.metadata?.rateConstants?.[rate]
                // Only render Line if the rate value is valid
                if (rateValue === undefined || rateValue === null || !isFinite(rateValue)) {
                  console.warn(`Skipping invalid rate ${rate}:`, rateValue)
                  return null
                }
                return (
                  <Line
                    key={rate}
                    type="monotone"
                    dataKey={rate}
                    stroke={colors[rateParameters.indexOf(rate) % colors.length]}
                    strokeWidth={3}
                    dot={false}
                    name={rate}
                    connectNulls
                    isAnimationActive={false}
                  />
                )
              })}
            </LineChart>
          </ResponsiveContainer>
          )}
        </div>

        {/* Info Box */}
        <div className="text-xs text-gray-600 bg-blue-50 border border-blue-200 rounded-lg p-3">
          <p className="font-semibold mb-1 flex items-center gap-2">
            <Lightbulb className="w-4 h-4" />
            About Reaction Rates:
          </p>
          <ul className="space-y-0.5 ml-4">
            <li>• <strong>PHOTO.*</strong> = Photolysis reactions (light-dependent)</li>
            <li>• <strong>USER.*</strong> = User-defined rate parameters</li>
            <li>• Rate values are constant for this simulation</li>
            <li>• Y-axis scale: <strong className="text-blue-600">{scaleType.toUpperCase()}</strong></li>
            <li>• Future versions will support time-varying rates</li>
          </ul>
        </div>
      </CardContent>
    </Card>
  )
}

export default ReactionRatesPlot
