import { BrowserRouter, Routes, Route, Navigate, useNavigate, useLocation } from 'react-router-dom'
import { useDispatch } from 'react-redux'
import { useEffect } from 'react'
import Navigation from '../Navigation'
import DashboardPage from './DashboardPage'
import GuidePage from './GuidePage'
import MechanismPage from './MechanismPage'
import ConditionsPage from './ConditionsPage'
import PlotsPage from './PlotsPage'
import { resetMechanism } from '../../redux/slices/mechanismSlice'
import { resetConditions } from '../../redux/slices/conditionsSlice'
import { resetSimulation } from '../../redux/slices/simulationSlice'

/**
 * AppContent Component
 * Handles auto-redirect to dashboard on app load
 */
function AppContent({ onNavigate }) {
  const dispatch = useDispatch()
  const navigate = useNavigate()
  const location = useLocation()

  // Reset state and redirect to dashboard on app entry
  useEffect(() => {
    // Always reset all Redux state to ensure fresh start
    dispatch(resetMechanism())
    dispatch(resetConditions())
    dispatch(resetSimulation())

    // Check if this is the first navigation by looking for a flag in sessionStorage
    const isFirstLoad = !sessionStorage.getItem('musicbox-app-loaded')

    if (isFirstLoad) {
      // Mark that the app has been loaded for navigation purposes
      sessionStorage.setItem('musicbox-app-loaded', 'true')

      // Navigate to dashboard if not already there
      if (location.pathname !== '/') {
        navigate('/', { replace: true })
      }
    }
  }, [])

  // Handle exit app - reset all Redux state and session before navigating home
  const handleExitApp = () => {
    dispatch(resetMechanism())
    dispatch(resetConditions())
    dispatch(resetSimulation())
    // Clear the session flag so next time user enters app, it goes to dashboard
    sessionStorage.removeItem('musicbox-app-loaded')
    onNavigate('home')
  }

  return (
    <div className="min-h-screen bg-gradient-to-bl from-[#141E30] via-[#1f2f45] to-[#35577D] relative">
      {/* Liquid glass background effects */}
      <div className="fixed inset-0 bg-[radial-gradient(ellipse_at_top_right,_var(--tw-gradient-stops))] from-[#35577D]/30 via-transparent to-transparent pointer-events-none"></div>
      <div className="fixed inset-0 bg-[radial-gradient(ellipse_at_bottom_left,_var(--tw-gradient-stops))] from-[#141E30]/40 via-transparent to-transparent pointer-events-none"></div>

      {/* Responsive sidebar navigation */}
      <Navigation onBackToHome={handleExitApp} />

      {/* Main content area with responsive left margin for sidebar */}
      <div className="min-h-screen relative z-10 lg:ml-64 transition-all duration-300">
        {/* Responsive container with proper padding for mobile */}
        <div className="container mx-auto px-2 xs:px-3 sm:px-6 lg:px-8 py-3 xs:py-4 sm:py-6 lg:py-8 pt-14 xs:pt-16 lg:pt-6">
          <Routes>
            <Route path="/" element={<DashboardPage />} />
            <Route path="/guide" element={<GuidePage />} />
            <Route path="/mechanism" element={<MechanismPage />} />
            <Route path="/conditions" element={<ConditionsPage />} />
            <Route path="/plots" element={<PlotsPage />} />
            <Route path="*" element={<Navigate to="/" replace />} />
          </Routes>
        </div>
      </div>
    </div>
  )
}

/**
 * MusicBoxAppNew Component
 * Main application with Redux-powered features
 */
function MusicBoxAppNew({ onNavigate }) {
  return (
    <BrowserRouter>
      <AppContent onNavigate={onNavigate} />
    </BrowserRouter>
  )
}

export default MusicBoxAppNew
