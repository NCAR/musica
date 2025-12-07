import * as React from "react"
import { cn } from "../../lib/utils"

const Badge = React.forwardRef(({ className, variant = "default", ...props }, ref) => {
  const variants = {
    default: "bg-blue-500/20 backdrop-blur-lg border border-blue-400/40 text-blue-300",
    success: "bg-green-500/20 backdrop-blur-lg border border-green-400/40 text-green-300",
    warning: "bg-orange-500/20 backdrop-blur-lg border border-orange-400/40 text-orange-300",
    error: "bg-red-500/20 backdrop-blur-lg border border-red-400/40 text-red-300",
    outline: "bg-white/0 backdrop-blur-lg border border-white/20 text-gray-300",
    secondary: "bg-white/10 backdrop-blur-lg border border-white/20 text-gray-300",
    idle: "bg-gray-500/20 backdrop-blur-lg border border-gray-400/40 text-gray-300",
    running: "bg-blue-500/20 backdrop-blur-lg border border-blue-400/40 text-blue-300",
    succeeded: "bg-green-500/20 backdrop-blur-lg border border-green-400/40 text-green-300",
    failed: "bg-red-500/20 backdrop-blur-lg border border-red-400/40 text-red-300",
  }

  return (
    <span
      className={cn(
        "inline-flex items-center rounded-full px-3 py-1 text-xs font-semibold transition-all duration-300",
        variants[variant],
        className
      )}
      ref={ref}
      {...props}
    />
  )
})

Badge.displayName = "Badge"

export { Badge }
