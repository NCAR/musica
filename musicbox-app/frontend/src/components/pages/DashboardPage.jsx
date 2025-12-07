import { useState, useRef } from 'react'
import { useDispatch } from 'react-redux'
import { useNavigate } from 'react-router-dom'
import { v4 as uuidv4 } from 'uuid'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../ui/card'
import { Button } from '../ui/button'
import ExampleLoader from '../ExampleLoader'
import SimulationStatus from '../SimulationStatus'
import CurrentExampleIndicator from '../CurrentExampleIndicator'
import {
  resetMechanism,
  setSelectedMechanism,
  setSpecies,
  setReactions,
  setCurrentExample
} from '../../redux/slices/mechanismSlice'
import {
  resetConditions,
  setDuration,
  setTimeStep,
  setOutputFrequency,
  setTemperature,
  setPressure,
  setConcentrations,
  loadConditions
} from '../../redux/slices/conditionsSlice'
import { useToast } from '@/hooks/use-toast'
import { Alert, AlertDescription, AlertTitle } from '../ui/alert'
import { Atom, Settings, BarChart3, Rocket, PenLine, FolderOpen, Library } from 'lucide-react'

// dashboard with quick actions and example loader
export function DashboardPage() {
  const dispatch = useDispatch()
  const navigate = useNavigate()
  const { toast } = useToast()
  const fileInputRef = useRef(null)
  const [uploadError, setUploadError] = useState(null)
  const [showConfirmation, setShowConfirmation] = useState(false)
  const [showExamples, setShowExamples] = useState(false)
  const [showSimulationStatus, setShowSimulationStatus] = useState(false)

  const handleStartFromScratch = () => {
    setShowConfirmation(true)
  }

  const confirmStartFromScratch = () => {
    dispatch(resetMechanism())
    dispatch(resetConditions())
    dispatch(setSelectedMechanism('custom'))
    // hide examples/status on fresh start
    setShowExamples(false)
    setShowSimulationStatus(false)
    toast({
      title: 'Started Fresh!',
      description: 'Add species in the Mechanism section.',
    })
    setShowConfirmation(false)
    navigate('/mechanism')
  }

  const handleLoadConfiguration = (event) => {
    const file = event.target.files?.[0]
    if (!file) return

    const reader = new FileReader()
    reader.onload = (e) => {
      try {
        const config = JSON.parse(e.target?.result)

        // validate config file
        if (!config.mechanism || !config.conditions) {
          throw new Error('Invalid configuration file format. Must contain mechanism and conditions.')
        }

        // load mechanism config
        // uploaded configs always use 'custom' type
        // actual name is just metadata
        dispatch(setSelectedMechanism('custom'))

        if (config.mechanism.species && Array.isArray(config.mechanism.species)) {
          // normalize to uppercase
          const normalizedSpecies = config.mechanism.species.map(sp => ({
            ...sp,
            name: sp.name.toUpperCase()
          }))
          dispatch(setSpecies(normalizedSpecies))
        }

        if (config.mechanism.reactions && Array.isArray(config.mechanism.reactions)) {
          // add IDs if missing
          // normalize reactant/product names to uppercase
          const reactionsWithIds = config.mechanism.reactions.map(reaction => ({
            ...reaction,
            id: reaction.id || uuidv4(),
            reactants: reaction.reactants?.map(r => ({ ...r, name: r.name.toUpperCase() })) || [],
            products: reaction.products?.map(p => ({ ...p, name: p.name.toUpperCase() })) || []
          }))
          dispatch(setReactions(reactionsWithIds))
        }

        // mark as uploaded config
        dispatch(setCurrentExample({
          id: 'uploaded',
          name: `Uploaded: ${file.name}`,
          description: 'Custom configuration uploaded from file'
        }))

        // load conditions
        if (config.conditions.basic) {
          if (config.conditions.basic.duration !== undefined) {
            dispatch(setDuration(config.conditions.basic.duration))
          }
          if (config.conditions.basic.timeStep !== undefined) {
            dispatch(setTimeStep(config.conditions.basic.timeStep))
          }
          if (config.conditions.basic.outputFrequency !== undefined) {
            dispatch(setOutputFrequency(config.conditions.basic.outputFrequency))
          }
        }

        if (config.conditions.initial) {
          if (config.conditions.initial.temperature !== undefined) {
            dispatch(setTemperature(config.conditions.initial.temperature))
          }
          if (config.conditions.initial.pressure !== undefined) {
            dispatch(setPressure(config.conditions.initial.pressure))
          }
          if (config.conditions.initial.concentrations) {
            // normalize concentration names to uppercase
            const normalizedConcentrations = {}
            Object.entries(config.conditions.initial.concentrations).forEach(([species, value]) => {
              normalizedConcentrations[species.toUpperCase()] = value
            })
            dispatch(setConcentrations(normalizedConcentrations))
          }
        }

        // load evolving conditions if present
        if (config.conditions.evolving) {
          // merge in evolving conditions
          dispatch(loadConditions({
            evolving: config.conditions.evolving
          }))
        }

        // hide examples/status when loading config
        setShowExamples(false)
        setShowSimulationStatus(false)

        toast({
          variant: 'success',
          title: 'Configuration Loaded Successfully!',
          description: `Loaded ${config.mechanism.species?.length || 0} species and ${config.mechanism.reactions?.length || 0} reactions from ${file.name}`,
        })

        // go to mechanism page to review
        navigate('/mechanism')
      } catch (err) {
        toast({
          title: 'Failed to Load Configuration',
          description: err.message || 'Failed to load configuration file. Please check the file format.',
          variant: 'destructive',
        })
      }
    }
    reader.readAsText(file)
  }

  return (
    <div className="space-y-4">
      {/* Welcome Section */}
      <Card>
        <CardHeader>
          <CardTitle className="text-lg xs:text-xl sm:text-2xl">MusicBox Interactive Dashboard</CardTitle>
          <CardDescription className="text-sm xs:text-base text-white-200 italic">
            Atmospheric Chemistry Simulation Platform powered by MUSICA/MICM
          </CardDescription>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 xs:grid-cols-2 md:grid-cols-3 gap-3 xs:gap-4">
            <div className="p-3 xs:p-4 bg-white/0 backdrop-blur-lg rounded-lg border border-white/20">
              <div className="mb-2">
                <Atom className="w-6 h-6 xs:w-7 xs:h-7 sm:w-8 sm:h-8" />
              </div>
              <h3 className="font-semibold mb-1 text-sm xs:text-base">Mechanism Editor</h3>
              <p className="text-xs text-gray-200 italic">
                Create and edit chemical species and reactions
              </p>
            </div>
            <div className="p-3 xs:p-4 bg-white/0 backdrop-blur-lg rounded-lg border border-white/20">
              <div className="mb-2">
                <Settings className="w-6 h-6 xs:w-7 xs:h-7 sm:w-8 sm:h-8" />
              </div>
              <h3 className="font-semibold mb-1 text-sm xs:text-base">Configure Conditions</h3>
              <p className="text-xs text-gray-200 italic">
                Set simulation parameters and initial conditions
              </p>
            </div>
            <div className="p-3 xs:p-4 bg-white/0 backdrop-blur-lg rounded-lg border border-white/20">
              <div className="mb-2">
                <BarChart3 className="w-6 h-6 xs:w-7 xs:h-7 sm:w-8 sm:h-8" />
              </div>
              <h3 className="font-semibold mb-1 text-sm xs:text-base">Visualize Results</h3>
              <p className="text-xs text-gray-200 italic">
                Analyze concentration profiles and reaction rates
              </p>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Getting Started Options */}
      <Card className="border-2 border-white/20">
        <CardHeader>
          <CardTitle className="flex items-center gap-2 text-lg xs:text-xl sm:text-2xl">
            <Rocket className="w-5 h-5 xs:w-6 xs:h-6" />
            Getting Started
          </CardTitle>
          <CardDescription className="text-white italic text-sm xs:text-base">Choose how you want to start using MusicBox</CardDescription>
        </CardHeader>
        <CardContent className="space-y-3">
          <div className="grid grid-cols-1 xs:grid-cols-2 md:grid-cols-3 gap-3">
            {/* Start from Scratch */}
            <div className="p-3 xs:p-4 bg-white/0 backdrop-blur-lg rounded-lg border-2 border-white/20">
              <div className="mb-2">
                <PenLine className="w-8 h-8 xs:w-9 xs:h-9 sm:w-10 sm:h-10" />
              </div>
              <h4 className="font-bold mb-2 text-sm xs:text-base">Start from Scratch</h4>
              <p className="text-xs text-gray-200 mb-3 italic">
                Build a custom mechanism from the ground up. Add species and reactions manually.
              </p>
              <Button
                onClick={handleStartFromScratch}
                variant="glass"
                className="w-full rounded-2xl border-2 text-xs xs:text-sm sm:text-base px-3 xs:px-4 py-2"
              >
                Create Custom
              </Button>
            </div>

            {/* Load Configuration */}
            <div className="p-3 xs:p-4 bg-white/0 backdrop-blur-lg rounded-lg border-2 border-white/20">
              <div className="mb-2">
                <FolderOpen className="w-8 h-8 xs:w-9 xs:h-9 sm:w-10 sm:h-10" />
              </div>
              <h4 className="font-bold mb-2 text-sm xs:text-base">Load Configuration</h4>
              <p className="text-xs text-gray-200 mb-3 xs:mb-7 italic">
                Load a previously saved configuration file (.json) to continue your work.
              </p>
              <input
                ref={fileInputRef}
                type="file"
                accept=".json"
                onChange={handleLoadConfiguration}
                className="hidden"
              />
              <Button
                variant="glass"
                className="w-full rounded-2xl border-2 cursor-pointer text-xs xs:text-sm sm:text-base px-3 xs:px-4 py-2"
                onClick={() => fileInputRef.current?.click()}
              >
                Upload Config
              </Button>
            </div>

            {/* Select Example */}
            <div className="p-3 xs:p-4 bg-white/0 backdrop-blur-lg rounded-lg border-2 border-white/20">
              <div className="mb-2">
                <Library className="w-8 h-8 xs:w-9 xs:h-9 sm:w-10 sm:h-10" />
              </div>
              <h4 className="font-bold mb-2 text-sm xs:text-base">Select Example</h4>
              <p className="text-xs text-gray-200 mb-3 italic">
                Choose from pre-configured examples (Chapman, TS1, Full Configuration) to get started quickly.
              </p>
              <Button
                variant="glass"
                className="w-full rounded-2xl border-2 text-xs xs:text-sm sm:text-base px-3 xs:px-4 py-2"
                onClick={() => {
                  const newShowState = !showExamples
                  setShowExamples(newShowState)
                  setShowSimulationStatus(newShowState)
                  if (newShowState) {
                    // Scroll to examples section after a short delay to allow rendering
                    setTimeout(() => {
                      document.getElementById('example-section')?.scrollIntoView({ behavior: 'smooth' })
                    }, 100)
                  }
                }}
              >
                {showExamples ? 'Hide Examples' : 'Browse Examples'}
              </Button>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Confirmation Alert */}
      {showConfirmation && (
        <Alert className="fixed top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 z-50 max-w-md glass border-2 shadow-2xl">
          <AlertTitle className="text-lg font-bold mb-2">Start from Scratch?</AlertTitle>
          <AlertDescription className="mb-4">
            This will clear any existing configuration. Are you sure you want to continue?
          </AlertDescription>
          <div className="flex gap-3 justify-end">
            <Button
              variant="glass"
              onClick={() => setShowConfirmation(false)}
              className="glass-button bg-red-600 text-white hover:bg-red-700"
            >
              Cancel
            </Button>
            <Button
              variant="glass"
              onClick={confirmStartFromScratch}
              className="bg-green-600 text-white hover:bg-green-700"
            >
              Yes, Start Fresh
            </Button>
          </div>
        </Alert>
      )}
      {showConfirmation && (
        <div className="fixed inset-0 bg-black/50 z-40" onClick={() => setShowConfirmation(false)} />
      )}

      {/* Current Example Indicator */}
      {/* <CurrentExampleIndicator /> */}

      {/* Simulation Status - Hidden by default, shown when Browse Examples is clicked */}
      {showSimulationStatus && <SimulationStatus />}

      {/* Example Loader - Hidden by default, shown when Browse Examples is clicked */}
      {showExamples && (
        <div id="example-section">
          <ExampleLoader />
        </div>
      )}

    </div>
  )
}

export default DashboardPage
