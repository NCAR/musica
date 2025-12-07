import { useState } from 'react'
import { useSelector, useDispatch } from 'react-redux'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../ui/card'
import { Button } from '../ui/button'
import { Lightbulb, Plus } from 'lucide-react'
import {
  setTemperature,
  setPressure,
  setConcentration,
  removeConcentration,
} from '../../redux/slices/conditionsSlice'
import { useToast } from '@/hooks/use-toast'

/**
 * InitialConditionsTab Component
 * Manages initial conditions (temperature, pressure, species concentrations)
 */
export function InitialConditionsTab() {
  const dispatch = useDispatch()
  const { toast } = useToast()
  const initial = useSelector((state) => state.conditions.initial)
  const selectedMechanism = useSelector((state) => state.mechanism.selectedMechanism)

  const [newSpecies, setNewSpecies] = useState('')
  const [newConcentration, setNewConcentration] = useState('')
  const [error, setError] = useState(null)

  const handleAddSpecies = () => {
    if (!newSpecies || !newConcentration) {
      setError('Please enter both species name and concentration')
      setTimeout(() => setError(null), 3000)
      toast({
        title: 'Error',
        description: 'Please enter both species name and concentration',
        variant: 'destructive',
      })
      return
    }

    // Automatically convert species name to uppercase to match mechanism species
    const normalizedSpecies = newSpecies.trim().toUpperCase()

    const value = parseFloat(newConcentration)
    if (isNaN(value)) {
      setError('Concentration must be a valid number')
      setTimeout(() => setError(null), 3000)
      toast({
        title: 'Error',
        description: 'Concentration must be a valid number',
        variant: 'destructive',
      })
      return
    }

    if (initial.concentrations[normalizedSpecies]) {
      setError(`Species "${normalizedSpecies}" already exists`)
      setTimeout(() => setError(null), 3000)
      toast({
        title: 'Error',
        description: `Species "${normalizedSpecies}" already exists`,
        variant: 'destructive',
      })
      return
    }

    dispatch(setConcentration({ species: normalizedSpecies, value }))
    toast({
      title: 'Species Added',
      description: `Successfully added ${normalizedSpecies} with concentration ${value}`,
      variant: 'success',
    })
    setNewSpecies('')
    setNewConcentration('')
  }

  const handleRemoveSpecies = (species) => {
    dispatch(removeConcentration(species))
    toast({
      title: 'Species Removed',
      description: `Removed ${species} from initial concentrations`,
      variant: 'delete',
    })
  }

  const handleConcentrationChange = (species, value) => {
    const parsed = parseFloat(value)
    if (!isNaN(parsed)) {
      dispatch(setConcentration({ species, value: parsed }))
    }
  }

  return (
    <div className="space-y-4">
      {/* Environmental Conditions */}
      <Card>
        <CardHeader>
          <CardTitle>Environmental Conditions</CardTitle>
          <CardDescription>Set initial temperature and pressure</CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
          <div>
            <label className="block text-sm font-semibold text-blue-100 mb-2">
              Temperature (K)
            </label>
            <input
              type="number"
              value={initial.temperature}
              onChange={(e) => {
                const value = parseFloat(e.target.value)
                if (!isNaN(value)) dispatch(setTemperature(value))
              }}
              step="0.1"
              min="0"
              className="w-full px-3 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent"
            />
            <p className="text-xs text-gray-500 mt-1">
              {(initial.temperature - 273.15).toFixed(2)}°C
            </p>
          </div>

          <div>
            <label className="block text-sm font-semibold text-blue-100 mb-2">
              Pressure (Pa)
            </label>
            <input
              type="number"
              value={initial.pressure}
              onChange={(e) => {
                const value = parseFloat(e.target.value)
                if (!isNaN(value)) dispatch(setPressure(value))
              }}
              step="100"
              min="0"
              className="w-full px-3 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent"
            />
            <p className="text-xs text-gray-500 mt-1">
              {(initial.pressure / 101325).toFixed(4)} atm
            </p>
          </div>
        </CardContent>
      </Card>

      {/* Species Concentrations */}
      <Card>
        <CardHeader>
          <CardTitle>Species Concentrations</CardTitle>
          <CardDescription>
            Set initial concentrations for chemical species
          </CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
          {/* Add New Species */}
          <div className="p-4 bg-white/0 backdrop-blur-lg rounded-xl border-2 border-white/20">
            <h4 className="font-bold text-sm mb-3 text-blue-100 flex items-center gap-2">
              <Plus className="w-4 h-4" />
              Add New Species
            </h4>

            {error && (
              <div className="bg-red-900/20 backdrop-blur-lg border border-red-400/30 text-red-700 px-3 py-2 rounded mb-3 text-xs">
                {error}
              </div>
            )}

            <div className="flex flex-col gap-3">
              <input
                type="text"
                placeholder="Species name (e.g., OH, NO3)"
                value={newSpecies}
                onChange={(e) => setNewSpecies(e.target.value)}
                className="px-3 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent"
              />
              <input
                type="text"
                placeholder="Concentration (e.g., 1e-8)"
                value={newConcentration}
                onChange={(e) => setNewConcentration(e.target.value)}
                className="px-3 py-2 border-2 border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded-lg text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent"
              />
            </div>

            <Button
              onClick={handleAddSpecies}
              variant="apple"
              size="default"
              className="mt-3 rounded-2xl"
            >
              Add Species
            </Button>
          </div>

          {/* Existing Species List */}
          <div className="space-y-2">
            {Object.entries(initial.concentrations).length === 0 ? (
              <p className="text-center text-gray-500 py-8">
                No species configured. Add species above.
              </p>
            ) : (
              Object.entries(initial.concentrations).map(([species, concentration]) => (
                <div
                  key={species}
                  className="flex items-center gap-3 p-3 border border-white/20 rounded-lg bg-white/5 hover:bg-white/10 transition-colors"
                >
                  <div className="flex-1">
                    <label className="block text-sm font-semibold text-blue-100 mb-1">
                      {species}
                    </label>
                    <input
                      type="text"
                      value={concentration}
                      onChange={(e) => handleConcentrationChange(species, e.target.value)}
                      className="w-full px-2 py-1 border border-white/30 bg-white/10 text-white placeholder:text-gray-400 rounded text-sm font-mono focus:outline-none focus:ring-2 focus:ring-blue-600"
                    />
                  </div>
                  <Button
                    variant="glass"
                    size="sm"
                    onClick={() => handleRemoveSpecies(species)}
                    className="rounded-lg text-red-600 hover:bg-red-900/20 backdrop-blur-lg"
                  >
                    Remove
                  </Button>
                </div>
              ))
            )}
          </div>
        </CardContent>
      </Card>

      <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-3 text-xs text-gray-300">
        <p className="font-semibold mb-1 flex items-center gap-2">
          <Lightbulb className="w-4 h-4" />
          Tips:
        </p>
        <ul className="space-y-0.5 ml-4">
          <li>• Use scientific notation for small values (e.g., 1e-8)</li>
          <li>• Concentrations are in mol/mol (mixing ratio)</li>
          <li>• Species must exist in the selected mechanism{selectedMechanism ? `: ${selectedMechanism.toUpperCase()}` : ''}</li>
        </ul>
      </div>
    </div>
  )
}

export default InitialConditionsTab
