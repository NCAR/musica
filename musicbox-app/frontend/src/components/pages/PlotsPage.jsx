import { useState } from 'react'
import { Card, CardContent } from '../ui/card'
import { Button } from '../ui/button'
import { SpeciesPlot, ReactionRatesPlot, EnvironmentPlot } from '../Plots'
import { Atom, FlaskConical, Thermometer } from 'lucide-react'

/**
 * PlotsPage Component
 * Multi-tab page for viewing different types of simulation results
 */
export function PlotsPage() {
  const [activeTab, setActiveTab] = useState('species') // 'species' | 'reactions' | 'environment'

  const tabs = [
    { id: 'species', label: 'Chemical Species', Icon: Atom, component: SpeciesPlot },
    { id: 'reactions', label: 'Reaction Rates', Icon: FlaskConical, component: ReactionRatesPlot },
    { id: 'environment', label: 'Environment', Icon: Thermometer, component: EnvironmentPlot },
  ]

  const ActiveComponent = tabs.find(t => t.id === activeTab)?.component

  return (
    <div className="space-y-4">
      {/* Tab Navigation */}
      <Card>
        <CardContent className="pt-4 xs:pt-5 sm:pt-6">
          <div className="flex flex-col xs:flex-row gap-2 border-b pb-2 overflow-x-auto">
            {tabs.map((tab) => {
              const IconComponent = tab.Icon
              return (
                <Button
                  key={tab.id}
                  variant={activeTab === tab.id ? 'default' : 'outline'}
                  onClick={() => setActiveTab(tab.id)}
                  className="rounded-2xl text-xs xs:text-sm sm:text-base px-3 xs:px-4 py-2 whitespace-nowrap flex-shrink-0"
                >
                  <IconComponent className="w-4 h-4 xs:w-5 xs:h-5 mr-1.5 xs:mr-2" />
                  <span className="hidden xs:inline">{tab.label}</span>
                  <span className="xs:hidden">{tab.label.split(' ')[0]}</span>
                </Button>
              )
            })}
          </div>
        </CardContent>
      </Card>

      {/* Active Tab Content */}
      {ActiveComponent && <ActiveComponent />}
    </div>
  )
}

export default PlotsPage
