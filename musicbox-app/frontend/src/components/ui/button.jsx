import * as React from "react"
import { cn } from "../../lib/utils"

const Button = React.forwardRef(({ className, variant = "glass", size = "default", ...props }, ref) => {
  const variants = {
    default: "bg-gradient-to-r from-[#141E30] to-[#35577D] text-white hover:from-[#1a2638] hover:to-[#3d6189] shadow-[0_4px_16px_0_rgba(20,30,48,0.4)] hover:shadow-[0_6px_20px_0_rgba(53,87,125,0.5)] transition-all duration-300",
    outline: "border-2 border-[#35577D]/50 bg-transparent text-white hover:bg-[#141E30]/30 hover:border-[#35577D] backdrop-blur-sm",
    ghost: "hover:bg-[#141E30]/20 text-white backdrop-blur-sm",
    link: "text-cyan-400 underline-offset-4 hover:underline hover:text-cyan-300",
    glass: "bg-white/10 backdrop-blur-md border border-white/20 text-white hover:bg-white/20 shadow-[0_4px_16px_0_rgba(255,255,255,0.1)]",
    destructive: "bg-gradient-to-r from-red-600 to-red-700 text-white hover:from-red-700 hover:to-red-800 shadow-[0_4px_16px_0_rgba(220,38,38,0.4)]",
    apple: "group relative overflow-hidden backdrop-blur-2xl bg-gradient-to-br from-[#141E30]/40 via-[#35577D]/30 to-[#141E30]/40 border-2 border-white/40 text-white hover:bg-white/30 hover:scale-105 font-bold shadow-2xl hover:shadow-[#35577D]/50 transition-all duration-500",
  }

  const sizes = {
    default: "h-10 px-4 py-2",
    sm: "h-9 px-3",
    lg: "h-11 px-8",
    icon: "h-10 w-10",
  }

  const isAppleVariant = variant === "apple"

  return (
    <button
      className={cn(
        "inline-flex items-center justify-center rounded-xl text-sm font-medium transition-colors focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-blue-600 disabled:pointer-events-none disabled:opacity-50",
        variants[variant],
        sizes[size],
        className
      )}
      ref={ref}
      {...props}
    >
      {isAppleVariant ? (
        <>
          <span className="relative z-10 drop-shadow-lg">{props.children}</span>
          {/* Liquid glass shimmer effect */}
          <div className="absolute inset-0 bg-gradient-to-br from-white/40 via-white/20 to-transparent opacity-60"></div>
          <div className="absolute inset-0 bg-gradient-to-tr from-[#141E30]/30 via-transparent to-[#35577D]/30 opacity-50 group-hover:opacity-70 transition-opacity duration-500"></div>
          {/* Animated shimmer */}
          <div className="absolute inset-0 -translate-x-full group-hover:translate-x-full transition-transform duration-1000 bg-gradient-to-r from-transparent via-white/40 to-transparent"></div>
        </>
      ) : props.children}
    </button>
  )
})

Button.displayName = "Button"

export { Button }
