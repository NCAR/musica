import { useState } from 'react'
import { NavLink } from 'react-router-dom'
import { Home, BookOpen, Atom, Settings, BarChart3, ArrowLeft } from 'lucide-react'
import RunSimulationButton from './RunSimulationButton'

/**
 * Navigation Component
 * Responsive sidebar navigation with mobile hamburger menu
 */
export function Navigation({ onBackToHome = null }) {
  const [isMobileMenuOpen, setIsMobileMenuOpen] = useState(false)

  const navLinks = [
    { to: '/', label: 'Dashboard', Icon: Home },
    { to: '/guide', label: 'Guide', Icon: BookOpen },
    { to: '/mechanism', label: 'Mechanism', Icon: Atom },
    { to: '/conditions', label: 'Conditions', Icon: Settings },
    { to: '/plots', label: 'Results', Icon: BarChart3 },
  ]

  const closeMobileMenu = () => setIsMobileMenuOpen(false)

  return (
    <>
      {/* Mobile Hamburger/Close Button with Animation and Position Transition */}
      <button
        onClick={() => setIsMobileMenuOpen(!isMobileMenuOpen)}
        className={`lg:hidden fixed z-50 p-2 rounded-lg bg-white/10 backdrop-blur-lg border border-white/30 text-white shadow-[0_8px_32px_0_rgba(31,38,135,0.37)] opacity-70 hover:opacity-100 transition-all duration-300 ${
          isMobileMenuOpen ? 'top-4 left-[200px] xs:left-[216px] sm:left-[232px] md:left-[240px]' : 'top-4 left-4'
        }`}
        aria-label={isMobileMenuOpen ? "Close menu" : "Open menu"}
      >
        {/* Animated Hamburger Icon */}
        <div className="w-4 h-4 flex flex-col justify-center items-center relative">
          {/* Top Line */}
          <span
            className={`block w-4 h-0.5 bg-white rounded-full transition-all duration-400 ease-in-out absolute ${
              isMobileMenuOpen ? 'rotate-45' : '-translate-y-2'
            }`}
          ></span>
          {/* Middle Line */}
          <span
            className={`block w-4 h-0.5 bg-white rounded-full transition-all duration-300 ease-in-out absolute ${
              isMobileMenuOpen ? 'opacity-0 scale-0' : 'opacity-100 scale-100'
            }`}
          ></span>
          {/* Bottom Line */}
          <span
            className={`block w-4 h-0.5 bg-white rounded-full transition-all duration-400 ease-in-out absolute ${
              isMobileMenuOpen ? '-rotate-45' : 'translate-y-2'
            }`}
          ></span>
        </div>
      </button>

      {/* Mobile Overlay */}
      {isMobileMenuOpen && (
        <div
          className="lg:hidden fixed inset-0 bg-black/50 z-40"
          onClick={closeMobileMenu}
        />
      )}

      {/* Sidebar Navigation */}
      <nav className={`
        fixed left-0 top-0 h-screen bg-gradient-to-b from-[#141E30] to-[#35577D] text-white shadow-2xl flex flex-col z-40
        w-[240px] xs:w-[256px] sm:w-[272px] md:w-[280px] lg:w-64
        transition-transform duration-300 ease-in-out
        ${isMobileMenuOpen ? 'translate-x-0' : '-translate-x-full lg:translate-x-0'}
      `}>
      {/* Logo Section */}
      <div className="p-4 sm:p-5 md:p-6 border-b border-[#35577D]">
        <h1 className="text-lg sm:text-xl md:text-2xl font-bold">MusicBox Interactive</h1>
        <p className="text-xs sm:text-sm md:text-base text-blue-200 mt-1 sm:mt-2">Atmospheric Chemistry Simulation</p>
      </div>

      {/* Navigation Links */}
      <div className="flex-1 py-4 sm:py-5 md:py-6 px-3 sm:px-4 space-y-2 overflow-y-auto">
        {navLinks.slice(0, 4).map((link) => {
          const IconComponent = link.Icon
          return (
            <NavLink
              key={link.to}
              to={link.to}
              end={link.to === '/'}
              onClick={closeMobileMenu}
              className={({ isActive }) =>
                `flex items-center space-x-3 px-3 sm:px-4 py-2.5 sm:py-3 rounded-xl font-medium text-sm sm:text-base transition-all duration-300 ${
                  isActive
                    ? 'bg-white/20 backdrop-blur-lg border border-white/30 text-white shadow-[0_8px_32px_0_rgba(255,255,255,0.1)] hover:bg-white/30'
                    : 'bg-white/5 backdrop-blur-sm border border-white/10 hover:bg-white/15 hover:border-white/20 hover:shadow-[0_4px_16px_0_rgba(255,255,255,0.05)]'
                }`
              }
            >
              <IconComponent className="w-5 h-5 sm:w-6 sm:h-6 flex-shrink-0" />
              <span className="truncate">{link.label}</span>
            </NavLink>
          )
        })}

        {/* Separator Line */}
        <div className="border-t border-white/20 my-3 sm:my-4"></div>

        {/* Run Simulation Button */}
        <div className="px-1 sm:px-2">
          <RunSimulationButton className="w-full text-sm sm:text-base" />
        </div>

        {/* Separator Line */}
        <div className="border-t border-white/20 my-3 sm:my-4"></div>

        {/* Results Link */}
        {navLinks.slice(4).map((link) => {
          const IconComponent = link.Icon
          return (
            <NavLink
              key={link.to}
              to={link.to}
              end={link.to === '/'}
              onClick={closeMobileMenu}
              className={({ isActive }) =>
                `flex items-center space-x-3 px-3 sm:px-4 py-2.5 sm:py-3 rounded-xl font-medium text-sm sm:text-base transition-all duration-300 ${
                  isActive
                    ? 'bg-white/20 backdrop-blur-lg border border-white/30 text-white shadow-[0_8px_32px_0_rgba(255,255,255,0.1)] hover:bg-white/30'
                    : 'bg-white/5 backdrop-blur-sm border border-white/10 hover:bg-white/15 hover:border-white/20 hover:shadow-[0_4px_16px_0_rgba(255,255,255,0.05)]'
                }`
              }
            >
              <IconComponent className="w-5 h-5 sm:w-6 sm:h-6 flex-shrink-0" />
              <span className="truncate">{link.label}</span>
            </NavLink>
          )
        })}
      </div>

      {/* Exit Button */}
      {onBackToHome && (
        <div className="p-3 sm:p-4 border-t border-white/10">
          <button
            onClick={() => {
              closeMobileMenu()
              onBackToHome()
            }}
            className="w-full px-4 py-2.5 sm:py-3 rounded-xl font-medium text-sm sm:text-base bg-red-500/10
             backdrop-blur-lg border border-red-400/60 hover:bg-red-500/40 hover:border-red-500 text-white transition-all duration-300 shadow-[0_8px_32px_0_rgba(239,68,68,0.2)] hover:shadow-[0_8px_32px_0_rgba(239,68,68,0.4)] flex items-center justify-center space-x-2"
          >
            <ArrowLeft className="w-5 h-5" />
            <span>Exit App</span>
          </button>
        </div>
      )}
    </nav>
    </>
  )
}

export default Navigation
