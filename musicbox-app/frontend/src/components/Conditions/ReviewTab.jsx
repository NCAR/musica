import { useSelector } from 'react-redux'
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../ui/card'
import { Button } from '../ui/button'
import { ClipboardList, Microscope, Settings, FlaskConical, Download, Copy, CheckCircle2, XCircle, AlertCircle, Lightbulb } from 'lucide-react'
import { useToast } from '@/hooks/use-toast'
import RunSimulationButton from '../RunSimulationButton'

/**
 * ReviewTab Component
 * Review and download complete MusicBox configuration
 */
export function ReviewTab() {
  const mechanism = useSelector((state) => state.mechanism)
  const conditions = useSelector((state) => state.conditions)
  const { toast } = useToast()

  // Build complete configuration object
  const configuration = {
    mechanism: {
      name: mechanism.selectedMechanism || 'custom',
      species: mechanism.species,
      reactions: mechanism.reactions,
      phases: mechanism.phases,
    },
    conditions: {
      basic: conditions.basic,
      initial: conditions.initial,
      evolving: conditions.evolving,
    },
    metadata: {
      created: new Date().toISOString(),
      version: '1.0.0',
    },
  }

  const handleDownloadJSON = () => {
    const blob = new Blob([JSON.stringify(configuration, null, 2)], {
      type: 'application/json',
    })
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a')
    a.href = url
    a.download = `musicbox-config-${mechanism.selectedMechanism}-${Date.now()}.json`
    document.body.appendChild(a)
    a.click()
    document.body.removeChild(a)
    URL.revokeObjectURL(url)
  }

  const handleCopyToClipboard = async () => {
    try {
      await navigator.clipboard.writeText(JSON.stringify(configuration, null, 2))
      toast({
        title: 'Copied to Clipboard!',
        description: 'Configuration has been copied to your clipboard.',
      })
    } catch (error) {
      toast({
        title: 'Copy Failed',
        description: 'Failed to copy configuration to clipboard.',
        variant: 'destructive',
      })
    }
  }

  return (
    <div className="space-y-4">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <ClipboardList className="w-5 h-5" />
            Configuration Review
          </CardTitle>
          <CardDescription>
            Review your complete MusicBox configuration before running the simulation
          </CardDescription>
        </CardHeader>

        <CardContent className="space-y-4">
          {/* Summary Statistics */}
          <div className="grid grid-cols-2 xs:grid-cols-2 sm:grid-cols-4 gap-2 xs:gap-3">
            <div className="bg-white/5 backdrop-blur-lg border border-white/20 rounded-lg p-2 xs:p-3">
              <div className="text-xs xs:text-sm text-white-600 font-semibold truncate">Mechanism</div>
              <div className="text-sm xs:text-base sm:text-lg font-bold text-red-600/70 truncate">
                {mechanism.selectedMechanism ? mechanism.selectedMechanism.toUpperCase() : 'NONE'}
              </div>
            </div>
            <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-2 xs:p-3">
              <div className="text-xs xs:text-sm text-white-600 font-semibold">Species</div>
              <div className="text-sm xs:text-base sm:text-lg font-bold text-green-600">
                {mechanism.species.length}
              </div>
            </div>
            <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-2 xs:p-3">
              <div className="text-xs xs:text-sm text-white-600 font-semibold">Reactions</div>
              <div className="text-sm xs:text-base sm:text-lg font-bold text-orange-600">
                {mechanism.reactions.length}
              </div>
            </div>
            <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-2 xs:p-3">
              <div className="text-xs xs:text-sm text-white-600 font-semibold">Duration</div>
              <div className="text-lg font-bold text-white-400">
                {(conditions.basic.duration / 3600).toFixed(1)}h
              </div>
            </div>
          </div>

          {/* Configuration Preview - List Format with Columns */}
          <div>
            <h4 className="font-semibold text-sm xs:text-base sm:text-lg mb-2 xs:mb-3">Configuration Data</h4>
            <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-2 xs:p-3 sm:p-4 max-h-96 overflow-auto">
              <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3 xs:gap-4">

                {/* Column 1: Mechanism Section */}
                <div className="sm:border-r border-white/20 sm:pr-4 pb-3 sm:pb-0 border-b sm:border-b-0">
                  <h5 className="font-semibold text-cyan-600 text-sm xs:text-base mb-2 flex items-center gap-2">
                    <Microscope className="w-3 h-3 xs:w-4 xs:h-4" />
                    Mechanism
                  </h5>
                  <ul className="ml-3 xs:ml-4 space-y-1 text-xs xs:text-sm text-gray-300">
                    <li><span className="font-semibold">Name:</span> <span className='font-medium'>{mechanism.selectedMechanism || 'custom'}</span></li>
                    <li><span className="font-semibold">Species Count:</span> <span className='font-medium'>{mechanism.species.length}</span></li>
                    <li><span className="font-semibold">Reactions Count:</span> <span className='font-medium'>{mechanism.reactions.length}</span></li>
                    <li><span className="font-semibold">Phases:</span> <span className='font-medium'>{mechanism.phases?.length || 0}</span></li>
                  </ul>
                </div>

                {/* Column 2: Conditions Section */}
                <div className="lg:border-r border-white/20 lg:pr-4 pb-3 lg:pb-0 border-b lg:border-b-0">
                  <h5 className="font-semibold text-green-600 text-sm xs:text-base mb-2 flex items-center gap-2">
                    <Settings className="w-3 h-3 xs:w-4 xs:h-4" />
                    Conditions
                  </h5>

                  {/* Basic Conditions */}
                  <div className="mb-3">
                    <h6 className="font-bold text-xs xs:text-sm text-gray-300 mb-1">Basic Settings:</h6>
                    <ul className="ml-3 xs:ml-4 space-y-1 text-xs xs:text-sm text-gray-300">
                      <li><span className="font-semibold">Temperature:</span> <span className='font-medium'>{conditions.basic.temperature} K</span></li>
                      <li><span className="font-semibold">Pressure:</span> <span className='font-medium'>{conditions.basic.pressure} Pa</span></li>
                      <li><span className="font-semibold">Duration:</span> <span className='font-medium'>{conditions.basic.duration} s ({(conditions.basic.duration / 3600).toFixed(2)} h)</span></li>
                      <li><span className="font-semibold">Time Step:</span> <span className='font-medium'>{conditions.basic.timeStep} s</span></li>
                    </ul>
                  </div>

                  {/* Evolving Conditions */}
                  {Object.keys(conditions.evolving || {}).length > 0 && (
                    <div>
                      <h6 className="font-bold text-xs xs:text-sm text-gray-300 mb-1">Evolving:</h6>
                      <ul className="ml-3 xs:ml-4 space-y-1 text-xs xs:text-sm text-gray-300">
                        {Object.entries(conditions.evolving).map(([key, value]) => (
                          <li key={key}>
                            <span className="font-semibold">{key}:</span> <span className='font-medium'>{JSON.stringify(value)}</span>
                          </li>
                        ))}
                      </ul>
                    </div>
                  )}
                </div>

                {/* Column 3: Initial Concentrations & Metadata */}
                <div>
                  {/* Initial Concentrations */}
                  <div className="mb-3 xs:mb-4">
                    <h5 className="font-bold text-orange-600 text-sm xs:text-base mb-2 flex items-center gap-2">
                      <FlaskConical className="w-3 h-3 xs:w-4 xs:h-4" />
                      Initial Concentrations
                    </h5>
                    <ul className="ml-3 xs:ml-4 space-y-1 text-xs xs:text-sm text-gray-300 max-h-48 overflow-y-auto">
                      {Object.keys(conditions.initial.concentrations).length > 0 ? (
                        Object.entries(conditions.initial.concentrations).map(([species, conc]) => (
                          <li key={species}>
                            <span className="font-semibold">{species}:</span> <span className='font-medium'>{conc}</span>
                          </li>
                        ))
                      ) : (
                        <li className="text-gray-400 italic">No initial concentrations</li>
                      )}
                    </ul>
                  </div>

                  {/* Metadata Section */}
                  <div className="border-t border-white/20 pt-2 xs:pt-3">
                    <h5 className="font-semibold text-white-600 text-sm xs:text-base mb-2">Metadata</h5>
                    <ul className="ml-3 xs:ml-4 space-y-1 text-xs xs:text-sm text-gray-300">
                      <li><span className="font-semibold">Version:</span> <span className='font-medium'>{configuration.metadata.version}</span></li>
                      <li><span className="font-semibold">Created:</span> <span className='font-medium'>{new Date(configuration.metadata.created).toLocaleString()}</span></li>
                    </ul>
                  </div>
                </div>

              </div>
            </div>
          </div>

          {/* Action Buttons */}
          <div className="flex flex-col xs:flex-row gap-2 xs:gap-3">
            <Button
              onClick={handleDownloadJSON}
              variant="glass"
              className="flex-1 rounded-2xl bg-gradient-to-r from-blue-500 to-indigo-600 hover:from-blue-600 hover:to-indigo-700 text-white font-semibold shadow-lg hover:shadow-xl transition-all duration-300 text-xs xs:text-sm sm:text-base px-3 xs:px-4 py-2 xs:py-2.5"
            >
              <Download className="w-3 h-3 xs:w-4 xs:h-4 mr-1.5 xs:mr-2" />
              <span className="truncate">Download Configuration</span>
            </Button>
            <Button
              onClick={handleCopyToClipboard}
              variant="glass"
              className="flex-1 rounded-2xl border-2 text-xs xs:text-sm sm:text-base px-3 xs:px-4 py-2 xs:py-2.5"
            >
              <Copy className="w-3 h-3 xs:w-4 xs:h-4 mr-1.5 xs:mr-2" />
              <span className="truncate">Copy to Clipboard</span>
            </Button>
          </div>
        </CardContent>
      </Card>

      {/* Configuration Checklist */}
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            Configuration Checklist
          </CardTitle>
          <CardDescription className="italic">Ensure your configuration is completed</CardDescription>
        </CardHeader>
        <CardContent>
          <div className="space-y-2 text-md">
            <div className={`flex items-center gap-2 ${mechanism.selectedMechanism ? 'text-green-400' : 'text-red-500'}`}>
              {mechanism.selectedMechanism ? <CheckCircle2 className="w-4 h-4" /> : <XCircle className="w-4 h-4" />}
              <span>Mechanism selected: {mechanism.selectedMechanism || 'None'}</span>
            </div>
            <div className={`flex items-center gap-2 ${mechanism.species.length > 0 ? 'text-green-400' : 'text-yellow-400'}`}>
              {mechanism.species.length > 0 ? <CheckCircle2 className="w-4 h-4" /> : <AlertCircle className="w-4 h-4" />}
              <span>Species defined: {mechanism.species.length}</span>
            </div>
            <div className={`flex items-center gap-2 ${mechanism.reactions.length > 0 ? 'text-green-400' : 'text-red-500'}`}>
              {mechanism.reactions.length > 0 ? <CheckCircle2 className="w-4 h-4" /> : <XCircle className="w-4 h-4" />}
              <span>Reactions defined: {mechanism.reactions.length}</span>
            </div>
            <div className={`flex items-center gap-2 ${conditions.basic.duration > 0 ? 'text-green-400' : 'text-red-500'}`}>
              {conditions.basic.duration > 0 ? <CheckCircle2 className="w-4 h-4" /> : <XCircle className="w-4 h-4" />}
              <span>Duration set: {(conditions.basic.duration / 3600).toFixed(1)} hours</span>
            </div>
            <div className={`flex items-center gap-2 ${Object.keys(conditions.initial.concentrations).length > 0 ? 'text-green-400' : 'text-yellow-400'}`}>
              {Object.keys(conditions.initial.concentrations).length > 0 ? <CheckCircle2 className="w-4 h-4" /> : <AlertCircle className="w-4 h-4" />}
              <span>Initial concentrations: {Object.keys(conditions.initial.concentrations).length}</span>
            </div>
          </div>
        </CardContent>
      </Card>

      <div className="bg-white/10 backdrop-blur-lg border border-white/20 rounded-lg p-3 text-xs text-gray-300">
        <p className="font-semibold mb-1 flex items-center gap-2">
          <Lightbulb className="w-4 h-4" />
          Next Steps:
        </p>
        <ul className="space-y-0.5 ml-4">
          <li>• Download your configuration to save it for later</li>
          <li>• Click "Run Simulation" to execute the model</li>
          <li>• View results in the Plots section after simulation completes</li>
          <li>• Configuration files can be reloaded from the Dashboard</li>
        </ul>
      </div>
    </div>
  )
}

export default ReviewTab
