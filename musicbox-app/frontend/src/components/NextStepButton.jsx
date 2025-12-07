import { useNavigate } from 'react-router-dom'
import { Button } from './ui/button'

/**
 * NextStepButton Component
 * Button to guide users through the workflow step by step
 * Can navigate to a different page OR trigger a callback (for tab changes)
 */
export function NextStepButton({ to = null, onClick = null, className = '', label = 'Next Step' }) {
  const navigate = useNavigate()

  const handleClick = () => {
    if (onClick) {
      // If a callback is provided, use it (for tab navigation)
      onClick()
    } else if (to) {
      // Otherwise navigate to the specified route
      navigate(to)
    }
  }

  return (
    <Button
      onClick={handleClick}
      variant="apple"
      size="sm"
      className={`rounded-2xl text-xs xs:text-sm sm:text-base px-3 xs:px-4 sm:px-6 py-1.5 xs:py-2 sm:py-2.5 ${className}`}
    >
      {label}
    </Button>
  )
}

export default NextStepButton
