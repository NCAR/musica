import { useState } from 'react'
import { Card, CardContent } from '../ui/card'
import { Button } from '../ui/button'
import { BasicConfigTab, InitialConditionsTab, EvolvingConditionsTab, ReviewTab } from '../Conditions'
import RunSimulationButton from '../RunSimulationButton'
import NextStepButton from '../NextStepButton'

/**
 * ConditionsPage Component
 * Main page for configuring simulation conditions with 4 tabs
 */
export function ConditionsPage() {
  const [activeTab, setActiveTab] = useState('basic') // 'basic' | 'initial' | 'evolving' | 'review'

  const tabs = [
    { id: 'basic', label: 'General', component: BasicConfigTab, nextTab: 'initial', nextLabel: 'Next to Initial Conditions' },
    { id: 'initial', label: 'Initial', component: InitialConditionsTab, nextTab: 'evolving', nextLabel: 'Next to Evolving Conditions' },
    { id: 'evolving', label: 'Evolving', component: EvolvingConditionsTab, nextTab: 'review', nextLabel: 'Next to Review Configuration' },
    { id: 'review', label: 'Review', component: ReviewTab },
  ]

  const ActiveComponent = tabs.find(t => t.id === activeTab)?.component
  const currentTab = tabs.find(t => t.id === activeTab)

  return (
    <div className="space-y-4">
      {/* Tab Navigation */}
      <Card>
        <CardContent className="pt-4 xs:pt-5 sm:pt-6">
          <div className="space-y-3 xs:space-y-4">
            {/* Tab Navigation */}
            <div className="flex gap-1.5 xs:gap-2 border-b border-white/20 pb-2 overflow-x-auto">
              {tabs.map((tab) => (
                <Button
                  key={tab.id}
                  variant={activeTab === tab.id ? 'default' : 'outline'}
                  onClick={() => setActiveTab(tab.id)}
                  className="rounded-2xl text-xs xs:text-sm sm:text-base px-2.5 xs:px-3 sm:px-4 py-1.5 xs:py-2 whitespace-nowrap flex-shrink-0"
                >
                  {tab.label}
                </Button>
              ))}
            </div>

            {/* Next Step Button (non-review tabs) or Run Simulation Button (review tab) */}
            <div className="flex justify-end">
              {activeTab !== 'review' ? (
                <NextStepButton
                  onClick={() => setActiveTab(currentTab?.nextTab)}
                  label={currentTab?.nextLabel || 'Next Step'}
                  className="w-full xs:w-auto"
                />
              ) : (
                <RunSimulationButton className="w-full xs:w-auto" />
              )}
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Active Tab Content */}
      {ActiveComponent && <ActiveComponent />}
    </div>
  )
}

export default ConditionsPage
