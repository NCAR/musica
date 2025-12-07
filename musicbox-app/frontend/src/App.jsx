import { useState } from 'react'
import HomePage from './components/pages/HomePage'
import AboutPage from './components/pages/AboutPage'
import ContactPage from './components/pages/ContactPage'
import MusicBoxAppNew from './components/pages/MusicBoxAppNew'
import { Toaster } from './components/ui/toaster'

/**
 * Main App Component
 * Handles navigation between marketing pages and application
 */
function App() {
  const [currentPage, setCurrentPage] = useState('home') // 'home', 'about', 'contact', 'app'
  const [appMountKey, setAppMountKey] = useState(0) // Counter to force remount

  const handleNavigate = (page) => {
    // If navigating to app, increment the key to force a fresh mount
    if (page === 'app') {
      setAppMountKey(prev => prev + 1)
    }
    setCurrentPage(page)
  }

  return (
    <>
      {currentPage === 'app' && <MusicBoxAppNew key={appMountKey} onNavigate={handleNavigate} />}
      {currentPage === 'about' && <AboutPage onNavigate={handleNavigate} />}
      {currentPage === 'contact' && <ContactPage onNavigate={handleNavigate} />}
      {currentPage === 'home' && <HomePage onNavigate={handleNavigate} />}
      <Toaster />
    </>
  )
}

export default App
