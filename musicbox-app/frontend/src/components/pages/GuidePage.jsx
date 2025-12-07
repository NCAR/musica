import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../ui/card'
import { Lightbulb, Save, Timer, BarChart3, RotateCw } from 'lucide-react'

/**
 * GuidePage Component
 * Provides step-by-step instructions for using MusicBox Interactive
 */
export function GuidePage() {
  return (
    <div className="space-y-6">
      {/* Welcome Section */}
      <Card>
        <CardHeader>
          <CardTitle className="text-3xl">Welcome to MusicBox Interactive</CardTitle>
          <CardDescription className="text-base text-white/80 italic">
            Your guide to setting up and running atmospheric chemistry simulations
          </CardDescription>
        </CardHeader>
      </Card>

      {/* Overview */}
      <Card>
        <CardHeader>
          <CardTitle>What is MusicBox Interactive?</CardTitle>
        </CardHeader>
        <CardContent className="space-y-3 text-sm">
          <p>
            MusicBox Interactive is a web-based platform for atmospheric chemistry modeling powered by
            <strong> MUSICA (Multi-Scale Infrastructure for Chemistry and Aerosols)</strong> and
            <strong> MICM (Modular Integrated Chemistry Module)</strong>.
          </p>
          <p>
            This application allows you to:
          </p>
          <ul className="list-disc list-inside ml-4 space-y-1">
            <li>Select from pre-configured chemical mechanisms (Chapman, TS1, Full Configuration)</li>
            <li>Define custom chemical species and reactions</li>
            <li>Configure atmospheric conditions and initial concentrations</li>
            <li>Run simulations and visualize concentration profiles over time</li>
            <li>Export results for further analysis</li>
          </ul>
        </CardContent>
      </Card>

      {/* Step-by-Step Guide with Vertical Progress */}
      <Card>
        <CardHeader>
          <CardTitle>Step-by-Step Workflow</CardTitle>
          <CardDescription className="text-white/80">Follow these steps to run your first simulation</CardDescription>
        </CardHeader>
        <CardContent>
          <div className="relative space-y-8">
            {/* Vertical Progress Line */}
            <div className="absolute left-5 top-8 bottom-2 w-0.5 bg-[linear-gradient(to_bottom,_#60a5fa,_#4ade80,_#fb923c,_#a78bfa,_#f472b6)] opacity-30"></div>

            {/* Step 1 */}
            <div className="flex gap-4 relative">
              <div className="flex-shrink-0 w-10 h-10 rounded-full bg-gradient-to-br from-blue-500 to-blue-700 text-white flex items-center justify-center font-bold shadow-lg border-2 border-white/30 backdrop-blur-lg z-10">
                1
              </div>
              <div className="flex-1">
                <h3 className="font-bold text-lg mb-2">Start from Dashboard</h3>
                <p className="text-sm text-gray-300 mb-2">
                  Choose one of three options to begin:
                </p>
                <ul className="list-disc list-inside ml-4 space-y-1 text-sm text-gray-300">
                  <li><strong>Browse Examples:</strong> Load a pre-configured mechanism (Chapman, TS1, or Full Configuration)</li>
                  <li><strong>Start from Scratch:</strong> Build a custom mechanism from the ground up</li>
                  <li><strong>Load Configuration:</strong> Upload a previously saved configuration file (.json)</li>
                </ul>
              </div>
            </div>

            {/* Step 2 */}
            <div className="flex gap-4 relative">
              <div className="flex-shrink-0 w-10 h-10 rounded-full bg-gradient-to-br from-green-500 to-green-700 text-white flex items-center justify-center font-bold shadow-lg border-2 border-white/30 backdrop-blur-lg z-10">
                2
              </div>
              <div className="flex-1">
                <h3 className="font-bold text-lg mb-2">Define Your Mechanism</h3>
                <p className="text-sm text-gray-300 mb-2">
                  Navigate to the <strong>Mechanism</strong> page to configure chemical species and reactions:
                </p>
                <ul className="list-disc list-inside ml-4 space-y-1 text-sm text-gray-300">
                  <li><strong>Species Tab:</strong> Add chemical species with properties (molecular weight, diffusion coefficient, etc.)</li>
                  <li><strong>Reactions Tab:</strong> Define chemical reactions with rate constants and reaction types (ARRHENIUS, PHOTOLYSIS, TROE, etc.)</li>
                </ul>
                <div className="mt-2 p-3 bg-white/0 backdrop-blur-lg border border-white/20 rounded-lg text-xs text-gray-300 w-fit">
                  <strong>Tip:</strong> If you loaded an example, species and reactions are already configured!
                </div>
              </div>
            </div>

            {/* Step 3 */}
            <div className="flex gap-4 relative">
              <div className="flex-shrink-0 w-10 h-10 rounded-full bg-gradient-to-br from-orange-500 to-orange-700 text-white flex items-center justify-center font-bold shadow-lg border-2 border-white/30 backdrop-blur-lg z-10">
                3
              </div>
              <div className="flex-1">
                <h3 className="font-bold text-lg mb-2">Configure Conditions</h3>
                <p className="text-sm text-gray-300 mb-2">
                  Navigate to the <strong>Conditions</strong> page and complete each tab:
                </p>
                <ul className="list-disc list-inside ml-4 space-y-1 text-sm text-gray-300">
                  <li><strong>General Tab:</strong> Set temperature, pressure, simulation duration, and time step</li>
                  <li><strong>Initial Tab:</strong> Define initial concentrations for each species</li>
                  <li><strong>Evolving Tab:</strong> (Optional) Configure time-varying conditions like temperature or emissions</li>
                  <li><strong>Review Tab:</strong> Review your complete configuration before running</li>
                </ul>
              </div>
            </div>

            {/* Step 4 */}
            <div className="flex gap-4 relative">
              <div className="flex-shrink-0 w-10 h-10 rounded-full bg-gradient-to-br from-purple-500 to-purple-700 text-white flex items-center justify-center font-bold shadow-lg border-2 border-white/30 backdrop-blur-lg z-10">
                4
              </div>
              <div className="flex-1">
                <h3 className="font-bold text-lg mb-2">Run Simulation</h3>
                <p className="text-sm text-gray-300 mb-2">
                  In the <strong>Review</strong> tab under Conditions, click the <strong>Run Simulation</strong> button:
                </p>
                <ul className="list-disc list-inside ml-4 space-y-1 text-sm text-gray-300">
                  <li>Review your configuration checklist to ensure all required fields are set</li>
                  <li>Click "Run Simulation" to execute the model</li>
                  <li>Wait for the simulation to complete (progress will be shown)</li>
                </ul>
              </div>
            </div>

            {/* Step 5 */}
            <div className="flex gap-4 relative">
              <div className="flex-shrink-0 w-10 h-10 rounded-full bg-gradient-to-br from-pink-500 to-pink-700 text-white flex items-center justify-center font-bold shadow-lg border-2 border-white/30 backdrop-blur-lg z-10">
                5
              </div>
              <div className="flex-1">
                <h3 className="font-bold text-lg mb-2">View Results</h3>
                <p className="text-sm text-gray-300 mb-2">
                  Navigate to the <strong>Results</strong> page to visualize and export your data:
                </p>
                <ul className="list-disc list-inside ml-4 space-y-1 text-sm text-gray-300">
                  <li>View interactive plots showing species concentrations over time</li>
                  <li>Select specific species to display on the chart</li>
                  <li>Download results as CSV or JSON for further analysis</li>
                  <li>Copy data to clipboard for quick access</li>
                </ul>
              </div>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Common Mechanisms */}
      <Card>
        <CardHeader>
          <CardTitle>Available Example Mechanisms</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
            <div className="p-4 bg-white/0 backdrop-blur-lg border border-white/20 rounded-lg">
              <h4 className="font-bold mb-2 underline decoration-clone">Chapman Mechanism</h4>
              <p className="text-xs text-white-200 mb-2">
                Stratospheric oxygen chemistry with photolysis reactions
              </p>
              <ul className="text-xs text-white-200 space-y-1">
                <li>4 species (O2, O, O1D, O3)</li>
                <li>Photolysis-driven reactions</li>
                <li>Ideal for learning basics</li>
              </ul>
            </div>

            <div className="p-4 bg-white/0 backdrop-blur-lg border border-white/20 rounded-lg">
              <h4 className="font-bold mb-2 underline decoration-clone">TS1 Mechanism</h4>
              <p className="text-xs text-white-200 mb-2">
                Tropospheric chemistry with 210 species
              </p>
              <ul className="text-xs text-white-200 space-y-1">
                <li>210 species</li>
                <li>534 reactions (ARRHENIUS, TROE)</li>
                <li>Comprehensive tropospheric chemistry</li>
              </ul>
            </div>

            <div className="p-4 bg-white/0 backdrop-blur-lg border border-white/20 rounded-lg">
              <h4 className="font-bold mb-2 underline decoration-clone">Full Configuration</h4>
              <p className="text-xs text-white-200 mb-2">
                Test mechanism with all reaction types
              </p>
              <ul className="text-xs text-white-200 space-y-1">
                <li>10 species</li>
                <li>11 reaction types demonstrated</li>
                <li>Great for testing features</li>
              </ul>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Tips and Best Practices */}
      <Card>
        <CardHeader>
          <CardTitle>Tips & Best Practices</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="space-y-3 text-sm">
            <div className="flex gap-3">
              <Lightbulb className="w-6 h-6 flex-shrink-0" />
              <div className="text-gray-300">
                <strong>Use Scientific Notation:</strong> For concentrations and rate constants, use scientific notation (e.g., 1.5e-8) for better precision
              </div>
            </div>
            <div className="flex gap-3">
              <Save className="w-6 h-6 flex-shrink-0" />
              <div className="text-gray-300">
                <strong>Save Your Work:</strong> Download your configuration from the Review tab before running simulations, so you can reload it later
              </div>
            </div>
            <div className="flex gap-3">
              <Timer className="w-6 h-6 flex-shrink-0" />
              <div className="text-gray-300">
                <strong>Start Small:</strong> For initial tests, use shorter simulation times and larger time steps to ensure everything works correctly
              </div>
            </div>
            <div className="flex gap-3">
              <BarChart3 className="w-6 h-6 flex-shrink-0" />
              <div className="text-gray-300">
                <strong>Review Configuration:</strong> Always check the configuration checklist in the Review tab before running to ensure all required fields are complete
              </div>
            </div>
            <div className="flex gap-3">
              <RotateCw className="w-6 h-6 flex-shrink-0" />
              <div className="text-gray-300">
                <strong>Experiment:</strong> Try different mechanisms, modify parameters, and see how results change - that's the best way to learn!
              </div>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Need Help */}
      <Card className="border-2 border-white/20">
        <CardHeader>
          <CardTitle>Need More Help?</CardTitle>
        </CardHeader>
        <CardContent>
          <p className="text-sm text-gray-300 mb-3">
            For more information about MUSICA and MICM, visit the official documentation:
          </p>
          <ul className="space-y-2 text-sm">
            <li>
              <a
                href="https://github.com/NCAR/musica"
                target="_blank"
                rel="noopener noreferrer"
                className="text-blue-400 hover:text-blue-300 font-medium underline"
              >
                MUSICA GitHub Repository
              </a>
            </li>
            <li>
              <a
                href="https://ncar.github.io/musica/"
                target="_blank"
                rel="noopener noreferrer"
                className="text-blue-400 hover:text-blue-300 font-medium underline"
              >
                MUSICA Documentation
              </a>
            </li>
          </ul>
        </CardContent>
      </Card>
    </div>
  )
}

export default GuidePage
