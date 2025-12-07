import { useSelector } from 'react-redux'
import { Badge } from './ui/badge'
import { CheckCircle2 } from 'lucide-react'

/**
 * CurrentExampleIndicator Component
 * Displays a compact badge showing which example is currently loaded
 */
export function CurrentExampleIndicator() {
  const currentExample = useSelector((state) => state.mechanism.currentExample)
  const selectedMechanism = useSelector((state) => state.mechanism.selectedMechanism)

  // Don't show anything if no example is loaded
  if (!currentExample) {
    return null
  }

  return (
    <div className="flex items-center gap-2 p-3 bg-green-500/10 backdrop-blur-lg border border-green-400/30 rounded-lg">
      <CheckCircle2 className="text-green-400 w-5 h-5" />
      <span className="text-sm text-gray-300">
        <strong className="text-white font-semibold">Loaded:</strong> {currentExample.name}
      </span>
      {selectedMechanism && (
        <Badge variant="success" className="ml-auto">
          {selectedMechanism.toUpperCase()}
        </Badge>
      )}
    </div>
  )
}

export default CurrentExampleIndicator
