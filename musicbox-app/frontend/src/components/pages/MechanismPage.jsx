import { useState } from 'react'
import { Card, CardContent } from '../ui/card'
import { Button } from '../ui/button'
import { SpeciesEditor, ReactionEditor } from '../Mechanism'
import NextStepButton from '../NextStepButton'

/**
 * MechanismPage Component
 * Main page for editing chemical mechanism (species and reactions)
 */
export function MechanismPage() {
  const [activeTab, setActiveTab] = useState('species') // 'species' | 'reactions'

  const tabs = [
    { id: 'species', label: 'Species', component: SpeciesEditor, nextTab: 'reactions', nextLabel: 'Next to Add Reactions' },
    { id: 'reactions', label: 'Reactions', component: ReactionEditor, nextTo: '/conditions', nextLabel: 'Next to Configure Conditions' },
  ]

  const ActiveComponent = tabs.find(t => t.id === activeTab)?.component
  const currentTab = tabs.find(t => t.id === activeTab)

  return (
    <div className="space-y-4">
      {/* Tab Navigation with Next Step Button */}
      <Card>
        <CardContent className="pt-4 xs:pt-5 sm:pt-6">
          <div className="flex flex-col xs:flex-row items-stretch xs:items-center justify-between gap-2 xs:gap-3 mb-2">
            <div className="flex gap-1.5 xs:gap-2 border-b pb-2 flex-1 overflow-x-auto">
              {tabs.map((tab) => (
                <Button
                  key={tab.id}
                  variant={activeTab === tab.id ? 'default' : 'glass'}
                  onClick={() => setActiveTab(tab.id)}
                  className="rounded-2xl text-xs xs:text-sm sm:text-base px-3 xs:px-4 sm:px-6 py-1.5 xs:py-2 whitespace-nowrap flex-shrink-0"
                >
                  {tab.label}
                </Button>
              ))}
            </div>

            {/* Next button: Species tab -> Reactions tab, Reactions tab -> Conditions page */}
            {currentTab?.nextTab ? (
              <NextStepButton
                onClick={() => setActiveTab(currentTab.nextTab)}
                label={currentTab.nextLabel}
                className="w-full xs:w-auto"
              />
            ) : (
              <NextStepButton
                to={currentTab?.nextTo}
                label={currentTab?.nextLabel || 'Next Step'}
                className="w-full xs:w-auto"
              />
            )}
          </div>
        </CardContent>
      </Card>

      {/* Active Tab Content */}
      {ActiveComponent && <ActiveComponent />}
    </div>
  )
}

export default MechanismPage
