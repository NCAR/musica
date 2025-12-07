import { useState, useRef } from 'react'
import { useSelector, useDispatch } from 'react-redux'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../ui/card'
import { Button } from '../ui/button'
import { AlertCircle, Plus, Upload, Download } from 'lucide-react'
import { useToast } from '@/hooks/use-toast'
import {
  setEvolvingEnabled,
  setEvolvingTimes,
  setEvolvingTemperature,
  setEvolvingPressure,
  setInterpolationMethod,
} from '../../redux/slices/conditionsSlice'

// manages time-varying environmental conditions
export function EvolvingConditionsTab() {
  const dispatch = useDispatch()
  const { toast } = useToast()
  const evolving = useSelector((state) => state.conditions.evolving)
  const basicConditions = useSelector((state) => state.conditions.basic)
  const initialConditions = useSelector((state) => state.conditions.initial)
  const [newTime, setNewTime] = useState('')
  const [newTemp, setNewTemp] = useState('')
  const [newPress, setNewPress] = useState('')
  const fileInputRef = useRef(null)

  // validation for time point coverage
  const hasTimeAtZero = evolving.times && evolving.times.includes(0)
  const hasTimeAtEnd = evolving.times && evolving.times.length > 0 && evolving.times[evolving.times.length - 1] >= basicConditions.duration
  const needsMorePoints = evolving.enabled && (!evolving.times || evolving.times.length === 0)

  const handleToggleEvolving = () => {
    const newState = !evolving.enabled
    dispatch(setEvolvingEnabled(newState))
    toast({
      title: newState ? 'Evolving Conditions Enabled' : 'Evolving Conditions Disabled',
      description: newState
        ? 'Temperature and pressure will vary over time based on configured time points'
        : 'Initial temperature and pressure will remain constant throughout the simulation',
      variant: newState ? 'success' : 'warning',
    })
  }

  const handleAddTimePoint = () => {
    if (!newTime || !newTemp || !newPress) {
      toast({
        title: 'Missing Fields',
        description: 'Please fill in time, temperature, and pressure values',
        variant: 'destructive',
      })
      return
    }

    const time = parseFloat(newTime)
    const temp = parseFloat(newTemp)
    const press = parseFloat(newPress)

    if (isNaN(time) || isNaN(temp) || isNaN(press)) {
      toast({
        title: 'Invalid Input',
        description: 'All values must be valid numbers',
        variant: 'destructive',
      })
      return
    }

    // check for duplicate
    if (evolving.times.includes(time)) {
      toast({
        title: 'Duplicate Time Point',
        description: `A time point already exists at ${time} seconds`,
        variant: 'destructive',
      })
      return
    }

    // add to arrays
    const newTimes = [...evolving.times, time].sort((a, b) => a - b)
    const insertIndex = newTimes.indexOf(time)

    const newTemps = [...evolving.temperature]
    newTemps.splice(insertIndex, 0, temp)

    const newPresses = [...evolving.pressure]
    newPresses.splice(insertIndex, 0, press)

    dispatch(setEvolvingTimes(newTimes))
    dispatch(setEvolvingTemperature(newTemps))
    dispatch(setEvolvingPressure(newPresses))

    toast({
      title: 'Time Point Added',
      description: `Added time point at ${time}s: ${temp}K, ${press}Pa`,
      variant: 'success',
    })

    // reset form
    setNewTime('')
    setNewTemp('')
    setNewPress('')
  }

  const handleRemoveTimePoint = (index) => {
    const removedTime = evolving.times[index]
    const newTimes = evolving.times.filter((_, i) => i !== index)
    const newTemps = evolving.temperature.filter((_, i) => i !== index)
    const newPresses = evolving.pressure.filter((_, i) => i !== index)

    dispatch(setEvolvingTimes(newTimes))
    dispatch(setEvolvingTemperature(newTemps))
    dispatch(setEvolvingPressure(newPresses))

    toast({
      title: 'Time Point Removed',
      description: `Removed time point at ${removedTime}s`,
      variant: 'delete',
    })
  }

  const handleAutoSuggestStart = () => {
    if (evolving.times.includes(0)) {
      toast({
        title: 'Time Point Already Exists',
        description: 'A time point at t=0 already exists',
        variant: 'warning',
      })
      return
    }

    const newTimes = [0, ...evolving.times].sort((a, b) => a - b)
    const insertIndex = newTimes.indexOf(0)

    const newTemps = [...evolving.temperature]
    newTemps.splice(insertIndex, 0, initialConditions.temperature)

    const newPresses = [...evolving.pressure]
    newPresses.splice(insertIndex, 0, initialConditions.pressure)

    dispatch(setEvolvingTimes(newTimes))
    dispatch(setEvolvingTemperature(newTemps))
    dispatch(setEvolvingPressure(newPresses))

    toast({
      title: 'Start Point Added',
      description: `Added time point at t=0s using initial conditions`,
      variant: 'success',
    })
  }

  const handleAutoSuggestEnd = () => {
    if (evolving.times.includes(basicConditions.duration)) {
      toast({
        title: 'Time Point Already Exists',
        description: `A time point at t=${basicConditions.duration}s already exists`,
        variant: 'warning',
      })
      return
    }

    const newTimes = [...evolving.times, basicConditions.duration].sort((a, b) => a - b)
    const insertIndex = newTimes.indexOf(basicConditions.duration)

    // use last temp/pressure or initial
    const lastTemp = evolving.temperature.length > 0
      ? evolving.temperature[evolving.temperature.length - 1]
      : initialConditions.temperature
    const lastPress = evolving.pressure.length > 0
      ? evolving.pressure[evolving.pressure.length - 1]
      : initialConditions.pressure

    const newTemps = [...evolving.temperature]
    newTemps.splice(insertIndex, 0, lastTemp)

    const newPresses = [...evolving.pressure]
    newPresses.splice(insertIndex, 0, lastPress)

    dispatch(setEvolvingTimes(newTimes))
    dispatch(setEvolvingTemperature(newTemps))
    dispatch(setEvolvingPressure(newPresses))

    toast({
      title: 'End Point Added',
      description: `Added time point at t=${basicConditions.duration}s`,
      variant: 'success',
    })
  }

  const handleCSVUpload = (event) => {
    const file = event.target.files?.[0]
    if (!file) return

    const reader = new FileReader()
    reader.onload = (e) => {
      try {
        const text = e.target?.result
        const lines = text.split('\n').filter(line => line.trim())

        // skip header if present
        const dataLines = lines[0].toLowerCase().includes('time') ? lines.slice(1) : lines

        const times = []
        const temps = []
        const presses = []

        dataLines.forEach((line, index) => {
          const values = line.split(',').map(v => v.trim())
          if (values.length >= 3) {
            const time = parseFloat(values[0])
            const temp = parseFloat(values[1])
            const press = parseFloat(values[2])

            if (!isNaN(time) && !isNaN(temp) && !isNaN(press)) {
              times.push(time)
              temps.push(temp)
              presses.push(press)
            }
          }
        })

        if (times.length === 0) {
          toast({
            title: 'Invalid CSV',
            description: 'No valid data found. Expected format: time,temperature,pressure',
            variant: 'destructive',
          })
          return
        }

        // sort by time
        const sorted = times.map((t, i) => ({ time: t, temp: temps[i], press: presses[i] }))
          .sort((a, b) => a.time - b.time)

        dispatch(setEvolvingTimes(sorted.map(s => s.time)))
        dispatch(setEvolvingTemperature(sorted.map(s => s.temp)))
        dispatch(setEvolvingPressure(sorted.map(s => s.press)))

        toast({
          title: 'CSV Imported Successfully',
          description: `Loaded ${sorted.length} time points from file`,
          variant: 'success',
        })
      } catch (error) {
        toast({
          title: 'Import Failed',
          description: error.message,
          variant: 'destructive',
        })
      }
    }

    reader.readAsText(file)
    event.target.value = '' // Reset input
  }

  const handleCSVExport = () => {
    if (evolving.times.length === 0) {
      toast({
        title: 'No Data to Export',
        description: 'Add time points before exporting',
        variant: 'warning',
      })
      return
    }

    // create csv content
    const header = 'time,temperature,pressure\n'
    const rows = evolving.times.map((time, i) =>
      `${time},${evolving.temperature[i]},${evolving.pressure[i]}`
    ).join('\n')

    const csvContent = header + rows

    // create download link
    const blob = new Blob([csvContent], { type: 'text/csv' })
    const url = window.URL.createObjectURL(blob)
    const link = document.createElement('a')
    link.href = url
    link.download = 'evolving_conditions.csv'
    document.body.appendChild(link)
    link.click()
    document.body.removeChild(link)
    window.URL.revokeObjectURL(url)

    toast({
      title: 'CSV Exported',
      description: `Exported ${evolving.times.length} time points`,
      variant: 'success',
    })
  }

  return (
    <div className="space-y-4">
      <Card>
        <CardHeader>
          <div className="flex items-center justify-between">
            <div>
              <CardTitle>Evolving Conditions</CardTitle>
              <CardDescription className="text-white-500 italic mt-1">
                Define time-varying temperature and pressure
              </CardDescription>
            </div>
            <Button
              variant={evolving.enabled ? 'default' : 'glass'}
              onClick={handleToggleEvolving}
              className="rounded-2xl"
            >
              {evolving.enabled ? 'Enabled ✓' : 'Disabled'}
            </Button>
          </div>
        </CardHeader>

        <CardContent className="space-y-4">
          {!evolving.enabled ? (
            <div className="text-center py-8 text-white-100">
              <p className="text-sm">Enable evolving conditions to define time-series data</p>
              <p className="text-xs mt-2">
                When disabled, initial conditions will be used throughout the simulation
              </p>
            </div>
          ) : (
            <>
              {/* Interpolation Method Selector */}
              <div className="p-4 bg-white/5 backdrop-blur-lg rounded-xl border border-white/20">
                <label className="block text-xs font-semibold text-blue-100 mb-2">
                  Interpolation Method
                </label>
                <select
                  value={evolving.interpolationMethod || 'linear'}
                  onChange={(e) => dispatch(setInterpolationMethod(e.target.value))}
                  className="w-full px-3 py-2 border-2 border-white/30 bg-white/10 text-white rounded-lg text-sm focus:outline-none focus:ring-2 focus:ring-blue-600"
                >
                  <option value="linear">Linear - Smooth transition between points</option>
                  <option value="step">Step - Hold value until next point</option>
                  <option value="cubic">Cubic - Smooth curves (future feature)</option>
                </select>
                <p className="text-xs text-gray-400 mt-2">
                  {evolving.interpolationMethod === 'linear' && 'Values are interpolated linearly between time points'}
                  {evolving.interpolationMethod === 'step' && 'Values change abruptly at each time point'}
                  {evolving.interpolationMethod === 'cubic' && 'Cubic spline interpolation (not yet implemented - defaults to linear)'}
                </p>
              </div>

              {/* CSV Import/Export */}
              <div className="flex gap-2">
                <input
                  type="file"
                  ref={fileInputRef}
                  onChange={handleCSVUpload}
                  accept=".csv"
                  className="hidden"
                />
                <Button
                  variant="glass"
                  size="sm"
                  onClick={() => fileInputRef.current?.click()}
                  className="rounded-lg flex-1"
                >
                  <Upload className="w-4 h-4 mr-2" />
                  Import CSV
                </Button>
                <Button
                  variant="glass"
                  size="sm"
                  onClick={handleCSVExport}
                  disabled={evolving.times.length === 0}
                  className="rounded-lg flex-1"
                >
                  <Download className="w-4 h-4 mr-2" />
                  Export CSV
                </Button>
              </div>

              {/* Add Time Point Form */}
              <div className="p-4 bg-white/0 backdrop-blur-lg rounded-xl border-2 border-white/20">
                <h4 className="font-bold text-sm mb-3 text-blue-100 flex items-center gap-2">
                  <Plus className="w-4 h-4" />
                  Add Time Point Manually
                </h4>
                <div className="grid grid-cols-3 gap-3">
                  <div>
                    <label className="block text-xs font-semibold text-blue-100 mb-1">
                      Time (seconds)
                    </label>
                    <input
                      type="number"
                      value={newTime}
                      onChange={(e) => setNewTime(e.target.value)}
                      placeholder="0"
                      className="w-full px-2 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600"
                    />
                  </div>
                  <div>
                    <label className="block text-xs font-semibold text-blue-100 mb-1">
                      Temperature (K)
                    </label>
                    <input
                      type="number"
                      value={newTemp}
                      onChange={(e) => setNewTemp(e.target.value)}
                      placeholder="298.15"
                      className="w-full px-2 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600"
                    />
                  </div>
                  <div>
                    <label className="block text-xs font-semibold text-blue-100 mb-1">
                      Pressure (Pa)
                    </label>
                    <input
                      type="number"
                      value={newPress}
                      onChange={(e) => setNewPress(e.target.value)}
                      placeholder="101325"
                      className="w-full px-2 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600"
                    />
                  </div>
                </div>
                <Button
                  onClick={handleAddTimePoint}
                  variant="apple"
                  size="default"
                  className="w-full mt-3 rounded-2xl"
                >
                  Add Time Point
                </Button>
              </div>

              {/* Validation Warnings and Auto-Suggest */}
              {evolving.times.length > 0 && (
                <div className="space-y-2">
                  {!hasTimeAtZero && (
                    <div className="bg-yellow-900/20 border border-yellow-400/50 rounded-lg p-3 flex items-start gap-3">
                      <AlertCircle className="w-5 h-5 text-yellow-400 flex-shrink-0 mt-0.5" />
                      <div className="flex-1">
                        <p className="text-sm font-semibold text-yellow-100">Missing Start Point</p>
                        <p className="text-xs text-yellow-200 mt-1">
                          Consider adding a time point at t=0s to define initial environmental conditions
                        </p>
                        <Button
                          variant="glass"
                          size="sm"
                          onClick={handleAutoSuggestStart}
                          className="mt-2 rounded-lg text-xs"
                        >
                          Add Start Point (t=0s)
                        </Button>
                      </div>
                    </div>
                  )}
                  {!hasTimeAtEnd && (
                    <div className="bg-yellow-900/20 border border-yellow-400/50 rounded-lg p-3 flex items-start gap-3">
                      <AlertCircle className="w-5 h-5 text-yellow-400 flex-shrink-0 mt-0.5" />
                      <div className="flex-1">
                        <p className="text-sm font-semibold text-yellow-100">Incomplete Coverage</p>
                        <p className="text-xs text-yellow-200 mt-1">
                          No time point at simulation end (t={basicConditions.duration}s). Conditions will be extrapolated.
                        </p>
                        <Button
                          variant="glass"
                          size="sm"
                          onClick={handleAutoSuggestEnd}
                          className="mt-2 rounded-lg text-xs"
                        >
                          Add End Point (t={basicConditions.duration}s)
                        </Button>
                      </div>
                    </div>
                  )}
                </div>
              )}

              {needsMorePoints && (
                <div className="bg-orange-900/20 border border-orange-400/50 rounded-lg p-3 flex items-start gap-3">
                  <AlertCircle className="w-5 h-5 text-orange-400 flex-shrink-0 mt-0.5" />
                  <div className="flex-1">
                    <p className="text-sm font-semibold text-orange-100">No Time Points Configured</p>
                    <p className="text-xs text-orange-200 mt-1">
                      Evolving conditions are enabled but no time points have been added. Add at least one time point to use this feature.
                    </p>
                  </div>
                </div>
              )}

              {/* Time Points Table */}
              <div>
                <h4 className="font-semibold text-sm mb-2">Configured Time Points</h4>
                {evolving.times.length === 0 ? (
                  <p className="text-center text-gray-500 py-8 text-sm">
                    No time points configured. Add your first time point above.
                  </p>
                ) : (
                  <div className="border border-white/20 rounded-lg overflow-hidden">
                    <table className="w-full text-sm">
                      <thead className="bg-white/10 backdrop-blur-lg border-b border-white/20">
                        <tr>
                          <th className="text-left px-4 py-2 font-semibold">Time (s)</th>
                          <th className="text-left px-4 py-2 font-semibold">Temperature (K)</th>
                          <th className="text-left px-4 py-2 font-semibold">Pressure (Pa)</th>
                          <th className="text-right px-4 py-2 font-semibold">Actions</th>
                        </tr>
                      </thead>
                      <tbody>
                        {evolving.times.map((time, index) => (
                          <tr key={index} className="border-b border-white/10 hover:bg-white/10">
                            <td className="px-4 py-2 font-mono">{time}</td>
                            <td className="px-4 py-2 font-mono">
                              {evolving.temperature[index]}
                              <span className="text-xs text-gray-500 ml-2">
                                ({(evolving.temperature[index] - 273.15).toFixed(1)}°C)
                              </span>
                            </td>
                            <td className="px-4 py-2 font-mono">
                              {evolving.pressure[index]}
                              <span className="text-xs text-gray-500 ml-2">
                                ({(evolving.pressure[index] / 101325).toFixed(2)} atm)
                              </span>
                            </td>
                            <td className="px-4 py-2 text-right">
                              <Button
                                variant="glass"
                                size="sm"
                                onClick={() => handleRemoveTimePoint(index)}
                                className="rounded-lg text-red-600 hover:bg-red-900/20 backdrop-blur-lg"
                              >
                                Remove
                              </Button>
                            </td>
                          </tr>
                        ))}
                      </tbody>
                    </table>
                  </div>
                )}
              </div>
            </>
          )}
        </CardContent>
      </Card>

      <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-3 text-xs text-white">
        <p className="font-semibold mb-1 flex items-center gap-2">
          <AlertCircle className="w-4 h-4" />
          Important Notes:
        </p>
        <ul className="space-y-0.5 ml-4">
          <li>• Time points will be interpolated linearly between defined values</li>
          <li>• First time point should typically be at t=0</li>
          <li>• Time points are automatically sorted chronologically</li>
          <li>• Evolving conditions override initial temperature/pressure settings</li>
        </ul>
      </div>
    </div>
  )
}

export default EvolvingConditionsTab
