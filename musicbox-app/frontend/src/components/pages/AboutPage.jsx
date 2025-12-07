import { Button } from '../ui/button'
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from '../ui/card'
import { useRef, useEffect } from 'react'

function AboutPage({ onNavigate }) {
  const carouselRef = useRef(null)
  const animationRef = useRef(null)
  const scrollPositionRef = useRef(0)

  // Sponsors data
  const sponsors = [
    { name: 'National Science Foundation', logo: '/logos/NSF.png', desc: 'Award NSF-AGS 19 41110' },
    { name: 'National Center for Atmospheric Research', logo: '/logos/NCAR.avif', desc: 'MUSICA & MICM Development' },
    { name: 'Barcelona Supercomputing Center', logo: '/logos/BSC.JPEG', desc: 'CAMP Development & MONARCH Integration' },
    { name: 'University of Illinois', logo: '/logos/Illinois.JPEG', desc: 'CAMP Co-Development & PartMC Integration' },
    { name: 'Texas A&M University', logo: '/logos/TAMU.JPEG', desc: 'MusicBox Interactive Desktop Development' },
    { name: 'European Union Horizon 2020', logo: '/logos/EuroPeanUnion.JPEG', desc: 'Marie Sklodowska-Curie Grant No 747048' },
  ]

  // Infinite smooth auto-scroll with requestAnimationFrame
  useEffect(() => {
    const container = carouselRef.current
    if (!container) return

    const cardWidth = 224 // 192px (w-48) + 32px (gap-8) = 224px
    const totalWidth = cardWidth * sponsors.length
    let lastTimestamp = 0

    const animate = (timestamp) => {
      if (!lastTimestamp) lastTimestamp = timestamp
      const deltaTime = timestamp - lastTimestamp
      lastTimestamp = timestamp

      // Smooth consistent scrolling using deltaTime
      scrollPositionRef.current += (deltaTime / 1000) * 50 // 50 pixels per second

      // Seamless loop: reset when reaching end of first set
      if (scrollPositionRef.current >= totalWidth) {
        scrollPositionRef.current -= totalWidth
      }

      container.scrollLeft = scrollPositionRef.current

      animationRef.current = requestAnimationFrame(animate)
    }

    animationRef.current = requestAnimationFrame(animate)

    return () => {
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current)
      }
    }
  }, [sponsors.length])
  return (
    <div className="min-h-screen bg-white">
      {/* Navigation Bar */}
      <nav className="bg-white/80 border-b shadow-sm sticky top-0 z-50">
        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between h-14 sm:h-16">
            <div className="flex items-center">
              <h1 className="text-xl sm:text-2xl font-bold text-[#141E30] truncate">MUSICBOX</h1>
            </div>
            <div className="flex items-center space-x-2 xs:space-x-4 sm:space-x-6 md:space-x-8">
              <button onClick={() => onNavigate('home')} className="text-[#141E30] hover:text-[#35577D] font-bold text-xs xs:text-sm sm:text-base">Home</button>
              <button onClick={() => onNavigate('about')} className="text-[#35577D] font-bold border-b-2 border-[#35577D] text-xs xs:text-sm sm:text-base">About</button>
              {/* <a href="#model" className="text-gray-700 hover:text-blue-900">Model</a> */}
              {/* <a href="#data" className="text-gray-700 hover:text-blue-900">Data</a> */}
              {/* <a href="#documentation" className="text-[#141E30] font-bold hover:text-[#35577D]">Documentation</a> */}
              <button onClick={() => onNavigate('contact')} className="text-[#141E30] hover:text-[#35577D] font-bold text-xs xs:text-sm sm:text-base">Contact</button>
            </div>
          </div>
        </div>
      </nav>

      {/* About Hero Section with 3D Atom Animation */}
      <div className="bg-gradient-to-r from-[#141E30] to-[#35577D] text-white py-8 xs:py-12 sm:py-16 md:py-20">
        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6 xs:gap-8 sm:gap-10 md:gap-12 items-center">
            {/* Left Side - Content */}
            <div>
              <h1 className="text-2xl xs:text-3xl sm:text-4xl md:text-5xl font-bold mb-3 xs:mb-4 sm:mb-6">About MusicBox</h1>
              <p className="text-base xs:text-lg sm:text-xl text-blue-100 mb-4 xs:mb-6 sm:mb-8">
                A atmospheric box model
              </p>

              {/* What is MusicBox Content */}
              <div className="space-y-2 xs:space-y-3 sm:space-y-4 text-blue-50">
                <p className="leading-relaxed text-xs xs:text-sm sm:text-base">
                  Simulate chamber or flow-tube experiments, recreate field observations, or evaluate the effect of new chemistry in an existing system.
                  MusicBox is designed to let you build the chemical sysem you want. You can simulate that system for the conditions you're interested in, and evaluate the results.
                  <br />
                  When you're ready, communicate your new chemical system to the community, so it can run in any perticipatring model.
                </p>
                <Button
                  size="lg"
                  className="group relative overflow-hidden backdrop-blur-2xl bg-white/20 border-2 border-white/60 text-white hover:bg-white/30 hover:scale-105 px-4 xs:px-6 sm:px-8 md:px-10 py-3 xs:py-4 sm:py-5 md:py-7 text-sm xs:text-base sm:text-lg md:text-xl font-bold shadow-2xl hover:shadow-white/50 transition-all duration-500 rounded-2xl"
                  onClick={() => onNavigate('app')}
                >
                  <span className="relative z-10 drop-shadow-lg">Launch MusicBox</span>
                  {/* Liquid glass shimmer effect */}
                  <div className="absolute inset-0 bg-gradient-to-br from-white/40 via-white/20 to-transparent opacity-60"></div>
                  <div className="absolute inset-0 bg-gradient-to-tr from-blue-400/30 via-transparent to-purple-400/30 opacity-50 group-hover:opacity-70 transition-opacity duration-500"></div>
                  {/* Animated shimmer */}
                  <div className="absolute inset-0 -translate-x-full group-hover:translate-x-full transition-transform duration-1000 bg-gradient-to-r from-transparent via-white/40 to-transparent"></div>
                  {/* Glass refraction effect */}
                  <div className="absolute top-0 left-0 right-0 h-1/2 bg-gradient-to-b from-white/30 to-transparent rounded-t-lg"></div>
              </Button>
              </div>
            </div>

            {/* Right Side - 3D Atom Animation */}
            <div className="flex items-center justify-center">
              <div className="relative w-48 h-48 xs:w-56 xs:h-56 sm:w-64 sm:h-64 md:w-72 md:h-72 lg:w-80 lg:h-80" style={{perspective: '1000px'}}>

                {/* Nucleus - 3D Sphere (Stationary - Outside rotating container) */}
                <div className="absolute inset-0 flex items-center justify-center z-20">
                  <div
                    className="w-16 h-16 rounded-full"
                    style={{
                      background: 'radial-gradient(circle at 35% 35%, #fef3c7, #fbbf24, #f59e0b, #d97706, #92400e)',
                      boxShadow: `
                        0 0 40px rgba(251, 191, 36, 0.8),
                        0 0 80px rgba(251, 191, 36, 0.4),
                        inset -12px -12px 20px rgba(0, 0, 0, 0.5),
                        inset 10px 10px 20px rgba(255, 255, 255, 0.3)
                      `,
                      animation: 'nucleus-pulse 2s infinite ease-in-out'
                    }}>
                  </div>
                </div>

                {/* Orbits and Electrons - Rotating Wrapper */}
                <div
                  className="absolute inset-0 flex items-center justify-center"
                  style={{
                    transformStyle: 'preserve-3d',
                    animation: 'atom-rotate-3d 20s infinite linear'
                  }}>

                  {/* Orbit 1 */}
                  <div
                    className="absolute w-64 h-64 rounded-full animate-orbit-ring-1"
                    style={{
                      transformStyle: 'preserve-3d',
                      transform: 'rotateY(65deg) rotateX(0deg)',
                      border: '1px solid transparent'
                    }}>
                    {/* Orbit pseudo-element for subtle ring */}
                    <div className="absolute inset-0 border border-cyan-400/20 rounded-full" style={{zIndex: -1}}></div>
                    {/* Electron 1 - 3D Sphere */}
                    <div
                      className="absolute w-4 h-4 rounded-full animate-electron-1"
                      style={{
                        top: '50%',
                        left: '50%',
                        marginTop: '-0.5rem',
                        marginLeft: '-0.5rem',
                        perspective: '600px',
                        transformStyle: 'preserve-3d',
                        animationDelay: '0s'
                      }}>
                      <div
                        className="w-full h-full rounded-full"
                        style={{
                          background: 'radial-gradient(circle at 30% 30%, #cffafe, #22d3ee, #0891b2, #0e7490)',
                          boxShadow: `
                            0 0 15px rgba(34, 211, 238, 0.8),
                            0 0 30px rgba(34, 211, 238, 0.4),
                            inset -4px -4px 6px rgba(0, 0, 0, 0.5),
                            inset 3px 3px 6px rgba(255, 255, 255, 0.4)
                          `,
                          transform: 'rotateX(25deg) rotateY(15deg)',
                          transformStyle: 'preserve-3d'
                        }}>
                      </div>
                    </div>
                  </div>

                  {/* Orbit 2 */}
                  <div
                    className="absolute w-64 h-64 rounded-full animate-orbit-ring-2"
                    style={{
                      transformStyle: 'preserve-3d',
                      transform: 'rotateY(65deg) rotateX(-54deg)',
                      border: '1px solid transparent'
                    }}>
                    <div className="absolute inset-0 border border-purple-400/20 rounded-full" style={{zIndex: -1}}></div>
                    {/* Electron 2 - 3D Sphere */}
                    <div
                      className="absolute w-4 h-4 rounded-full animate-electron-2"
                      style={{
                        top: '50%',
                        left: '50%',
                        marginTop: '-0.5rem',
                        marginLeft: '-0.5rem',
                        perspective: '600px',
                        transformStyle: 'preserve-3d',
                        animationDelay: '-0.8s'
                      }}>
                      <div
                        className="w-full h-full rounded-full"
                        style={{
                          background: 'radial-gradient(circle at 30% 30%, #f3e8ff, #c084fc, #9333ea, #7e22ce)',
                          boxShadow: `
                            0 0 15px rgba(192, 132, 252, 0.8),
                            0 0 30px rgba(192, 132, 252, 0.4),
                            inset -4px -4px 6px rgba(0, 0, 0, 0.5),
                            inset 3px 3px 6px rgba(255, 255, 255, 0.4)
                          `,
                          transform: 'rotateX(25deg) rotateY(15deg)',
                          transformStyle: 'preserve-3d'
                        }}>
                      </div>
                    </div>
                  </div>

                  {/* Orbit 3 */}
                  <div
                    className="absolute w-64 h-64 rounded-full animate-orbit-ring-3"
                    style={{
                      transformStyle: 'preserve-3d',
                      transform: 'rotateY(65deg) rotateX(54deg)',
                      border: '1px solid transparent'
                    }}>
                    <div className="absolute inset-0 border border-pink-400/20 rounded-full" style={{zIndex: -1}}></div>
                    {/* Electron 3 - 3D Sphere */}
                    <div
                      className="absolute w-4 h-4 rounded-full animate-electron-3"
                      style={{
                        top: '50%',
                        left: '50%',
                        marginTop: '-0.5rem',
                        marginLeft: '-0.5rem',
                        perspective: '600px',
                        transformStyle: 'preserve-3d',
                        animationDelay: '-1.5s'
                      }}>
                      <div
                        className="w-full h-full rounded-full"
                        style={{
                          background: 'radial-gradient(circle at 30% 30%, #fce7f3, #f472b6, #ec4899, #db2777)',
                          boxShadow: `
                            0 0 15px rgba(244, 114, 182, 0.8),
                            0 0 30px rgba(244, 114, 182, 0.4),
                            inset -4px -4px 6px rgba(0, 0, 0, 0.5),
                            inset 3px 3px 6px rgba(255, 255, 255, 0.4)
                          `,
                          transform: 'rotateX(25deg) rotateY(15deg)',
                          transformStyle: 'preserve-3d'
                        }}>
                      </div>
                    </div>
                  </div>

                  {/* Orbit 4 */}
                  <div
                    className="absolute w-64 h-64 rounded-full animate-orbit-ring-4"
                    style={{
                      transformStyle: 'preserve-3d',
                      transform: 'rotateY(65deg) rotateX(108deg)',
                      border: '1px solid transparent'
                    }}>
                    <div className="absolute inset-0 border border-emerald-400/20 rounded-full" style={{zIndex: -1}}></div>
                    {/* Electron 4 - 3D Sphere */}
                    <div
                      className="absolute w-4 h-4 rounded-full animate-electron-4"
                      style={{
                        top: '50%',
                        left: '50%',
                        marginTop: '-0.5rem',
                        marginLeft: '-0.5rem',
                        perspective: '600px',
                        transformStyle: 'preserve-3d',
                        animationDelay: '-2.2s'
                      }}>
                      <div
                        className="w-full h-full rounded-full"
                        style={{
                          background: 'radial-gradient(circle at 30% 30%, #d1fae5, #34d399, #10b981, #059669)',
                          boxShadow: `
                            0 0 15px rgba(52, 211, 153, 0.8),
                            0 0 30px rgba(52, 211, 153, 0.4),
                            inset -4px -4px 6px rgba(0, 0, 0, 0.5),
                            inset 3px 3px 6px rgba(255, 255, 255, 0.4)
                          `,
                          transform: 'rotateX(25deg) rotateY(15deg)',
                          transformStyle: 'preserve-3d'
                        }}>
                      </div>
                    </div>
                  </div>

                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Warning Banner */}
      <div className="bg-yellow-50 border-l-4 border-yellow-400 p-3 xs:p-4 sm:p-6">
        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8">
          <div className="flex">
            <div className="flex-shrink-0">
              <svg className="h-4 w-4 xs:h-5 xs:w-5 text-yellow-400" viewBox="0 0 20 20" fill="currentColor">
                <path fillRule="evenodd" d="M8.257 3.099c.765-1.36 2.722-1.36 3.486 0l5.58 9.92c.75 1.334-.213 2.98-1.742 2.98H4.42c-1.53 0-2.493-1.646-1.743-2.98l5.58-9.92zM11 13a1 1 0 11-2 0 1 1 0 012 0zm-1-8a1 1 0 00-1 1v3a1 1 0 002 0V6a1 1 0 00-1-1z" clipRule="evenodd" />
              </svg>
            </div>
            <div className="ml-2 xs:ml-3">
              <p className="text-xs xs:text-sm text-yellow-700">
                <strong>Note:</strong> MusicBox is currently being tested and is under active development.
                Features and documentation are continuously being updated.
              </p>
            </div>
          </div>
        </div>
      </div>


      {/* Behind the Music Section */}
      <div className="bg-gradient-to-br from-[#141E30] via-[#1f2f45] to-[#35577D] py-8 xs:py-12 sm:py-16 relative overflow-hidden">
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_top_right,_var(--tw-gradient-stops))] from-[#35577D]/30 via-transparent to-transparent pointer-events-none"></div>
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_bottom_left,_var(--tw-gradient-stops))] from-[#141E30]/40 via-transparent to-transparent pointer-events-none"></div>

        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8 relative z-10">
          <h2 className="text-2xl xs:text-3xl sm:text-4xl font-bold text-center mb-2 xs:mb-3 sm:mb-4 text-white">Behind the Music</h2>
          <p className="text-center text-gray-300 mb-8 xs:mb-12 sm:mb-16 text-sm xs:text-base sm:text-lg">Powered by cutting-edge atmospheric chemistry frameworks</p>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-6 xs:gap-8 sm:gap-10 mb-8 xs:mb-10 sm:mb-12">
            {/* CAMP Card */}
            <div className="group relative overflow-hidden rounded-xl xs:rounded-2xl backdrop-blur-xl bg-white/5 border border-white/20 shadow-2xl">
              <div className="absolute inset-0 bg-gradient-to-br from-[#35577D]/20 via-transparent to-transparent opacity-50"></div>
              <div className="relative">
                <div className="bg-gradient-to-r from-[#35577D]/80 to-[#141E30]/80 backdrop-blur-lg text-white px-3 xs:px-4 sm:px-6 py-2 xs:py-3 sm:py-4 border-b border-white/20">
                  <h3 className="text-xl xs:text-2xl sm:text-3xl font-bold">CAMP</h3>
                  <p className="text-blue-200 text-sm xs:text-base sm:text-lg mt-0.5 xs:mt-1">Chemistry Across Multiple Phases</p>
                </div>
                <div className="p-4 xs:p-6 sm:p-8">
                  <p className="text-gray-300 mb-4 xs:mb-5 sm:mb-6 leading-relaxed text-xs xs:text-sm sm:text-base">
                    A run-time configured chemical system solver for gas- and condensed-phase reactions.
                    CAMP provides flexibility and performance for complex atmospheric chemistry simulations.
                  </p>
                  <div className="bg-white/5 backdrop-blur-lg border border-white/20 p-2 xs:p-3 sm:p-4 rounded-lg mb-3 xs:mb-4">
                    <p className="text-xs xs:text-sm text-gray-300">
                      <strong className="text-white font-semibold">Developed by:</strong> Barcelona Supercomputing Center (BSC) and University of Illinois
                    </p>
                  </div>
                  <Button variant="apple" size="default" className="rounded-2xl text-xs xs:text-sm sm:text-base">
                    Explore CAMP
                  </Button>
                </div>
              </div>
            </div>

            {/* MICM Card */}
            <div className="group relative overflow-hidden rounded-xl xs:rounded-2xl backdrop-blur-xl bg-white/5 border border-white/20 shadow-2xl ">
              <div className="absolute inset-0 bg-gradient-to-br from-[#141E30]/20 via-transparent to-transparent opacity-50 "></div>
              <div className="relative">
                <div className="bg-gradient-to-r from-[#141E30]/80 to-[#35577D]/80 backdrop-blur-lg text-white px-3 xs:px-4 sm:px-6 py-2 xs:py-3 sm:py-4 border-b border-white/20">
                  <h3 className="text-xl xs:text-2xl sm:text-3xl font-bold">MICM</h3>
                  <p className="text-green-200 text-sm xs:text-base sm:text-lg mt-0.5 xs:mt-1">Model-Independent Chemistry Module</p>
                </div>
                <div className="p-4 xs:p-6 sm:p-8">
                  <p className="text-gray-300 mb-4 xs:mb-5 sm:mb-6 leading-relaxed text-xs xs:text-sm sm:text-base">
                    Designed to bring flexibility for chemical systems to high-performance weather and climate models.
                    MICM enables efficient chemistry integration across multiple modeling platforms.
                  </p>
                  <div className="bg-white/5 backdrop-blur-lg border border-white/20 p-2 xs:p-3 sm:p-4 rounded-lg mb-3 xs:mb-4">
                    <p className="text-xs xs:text-sm text-gray-300">
                      <strong className="text-white font-semibold">Developed by:</strong> National Center for Atmospheric Research (NCAR)
                    </p>
                  </div>
                  <Button variant="apple" size="default" className="rounded-2xl text-xs xs:text-sm sm:text-base mt-2 xs:mt-3 sm:mt-4">
                    Explore MICM
                  </Button>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Overview Section */}
      <div className="bg-gradient-to-br from-[#141E30] via-[#1f2f45] to-[#35577D] py-8 xs:py-12 sm:py-16 relative overflow-hidden">
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_top_left,_var(--tw-gradient-stops))] from-[#141E30]/40 via-transparent to-transparent pointer-events-none"></div>
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_bottom_right,_var(--tw-gradient-stops))] from-[#35577D]/30 via-transparent to-transparent pointer-events-none"></div>

        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8 relative z-10">
          <h2 className="text-2xl xs:text-3xl font-bold text-center text-white mb-6 xs:mb-8 sm:mb-12">Overview</h2>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4 xs:gap-6 sm:gap-8">
            {/* Left Column - MUSICA */}
            <div className="group relative overflow-hidden rounded-xl xs:rounded-2xl backdrop-blur-xl bg-white/5 border border-white/20 shadow-2xl hover:shadow-[#35577D]/60 p-4 xs:p-6 sm:p-8 transition-all duration-500 hover:scale-105">
              <div className="absolute inset-0 bg-gradient-to-br from-[#35577D]/20 via-transparent to-transparent opacity-50 group-hover:opacity-70 transition-opacity duration-500"></div>
              <div className="relative">
                <h3 className="text-xl xs:text-2xl sm:text-3xl font-bold mb-4 xs:mb-5 sm:mb-6 text-white">MUSICA</h3>
                <div className="space-y-2 xs:space-y-3 sm:space-y-4 text-gray-300">
                  <p className="leading-relaxed text-xs xs:text-sm sm:text-base">
                    MusicBox is a component of MUSICA (Multi-Scale Infrastructure for Chemistry and Aerosols) at the National Center for
                    Atmospheric Research's Atmospheric Chemistry Observations and Modeling Laboratory.
                  </p>
                  <p className="leading-relaxed text-xs xs:text-sm sm:text-base">
                    MUSICA will become a computationally feasible global modeling framework that allows for the simulation of large-scale
                    atmospheric phenomena, while still resolving chemistry at emission and exposure relevant scales (down to ~4 km within the
                    next 5 years).
                  </p>
                  <p className="leading-relaxed text-xs xs:text-sm sm:text-base">
                    The Model-Independent Chemistry Module (MICM) is also developed at NCAR-ACOM and can bring MusicBox mechanisms
                    you develop here to high-performance weather and climate models.
                  </p>
                </div>
              </div>
            </div>

            {/* Right Column - Sponsor Info Cards */}
            <div className="space-y-2 xs:space-y-3 sm:space-y-4">
              {/* Barcelona Supercomputing Center */}
              <div className="group relative overflow-hidden rounded-lg backdrop-blur-lg bg-white/5 border border-white/20 shadow-lg hover:shadow-[#35577D]/40 p-3 xs:p-4 sm:p-6 transition-all duration-300 hover:scale-105">
                <div className="absolute inset-0 bg-gradient-to-br from-[#141E30]/20 via-transparent to-transparent opacity-40 group-hover:opacity-60 transition-opacity duration-300"></div>
                <div className="relative">
                  <h4 className="text-base xs:text-lg sm:text-xl font-bold mb-2 xs:mb-3 text-white">Barcelona Supercomputing Center (BSC)</h4>
                  <p className="text-xs xs:text-sm text-gray-300 leading-relaxed">
                    Earth Sciences Department: CAMP development and MONARCH model integration
                  </p>
                </div>
              </div>

              {/* University of Illinois */}
              <div className="group relative overflow-hidden rounded-lg backdrop-blur-lg bg-white/5 border border-white/20 shadow-lg hover:shadow-[#35577D]/40 p-3 xs:p-4 sm:p-6 transition-all duration-300 hover:scale-105">
                <div className="absolute inset-0 bg-gradient-to-br from-[#35577D]/20 via-transparent to-transparent opacity-40 group-hover:opacity-60 transition-opacity duration-300"></div>
                <div className="relative">
                  <h4 className="text-base xs:text-lg sm:text-xl font-bold mb-2 xs:mb-3 text-white">University of Illinois at Urbana-Champaign</h4>
                  <p className="text-xs xs:text-sm text-gray-300 leading-relaxed">
                    CAMP co-development and PartMC integration for atmospheric aerosol simulation
                  </p>
                </div>
              </div>

              {/* European Union Horizon 2020 */}
              <div className="group relative overflow-hidden rounded-lg backdrop-blur-lg bg-white/5 border border-white/20 shadow-lg hover:shadow-[#35577D]/40 p-3 xs:p-4 sm:p-6 transition-all duration-300 hover:scale-105">
                <div className="absolute inset-0 bg-gradient-to-br from-[#141E30]/20 via-transparent to-transparent opacity-40 group-hover:opacity-60 transition-opacity duration-300"></div>
                <div className="relative">
                  <h4 className="text-base xs:text-lg sm:text-xl font-bold mb-2 xs:mb-3 text-white">European Union Horizon 2020</h4>
                  <p className="text-xs xs:text-sm text-gray-300 leading-relaxed">
                    Marie Sklodowska-Curie grant agreement No 747048
                  </p>
                </div>
              </div>

              {/* National Science Foundation */}
              <div className="group relative overflow-hidden rounded-lg backdrop-blur-lg bg-white/5 border border-white/20 shadow-lg hover:shadow-[#35577D]/40 p-3 xs:p-4 sm:p-6 transition-all duration-300 hover:scale-105">
                <div className="absolute inset-0 bg-gradient-to-br from-[#35577D]/20 via-transparent to-transparent opacity-40 group-hover:opacity-60 transition-opacity duration-300"></div>
                <div className="relative">
                  <h4 className="text-base xs:text-lg sm:text-xl font-bold mb-2 xs:mb-3 text-white">National Science Foundation</h4>
                  <p className="text-xs xs:text-sm text-gray-300 leading-relaxed">
                    Award NSF-AGS 19 41110
                  </p>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Collaboration & Sponsors - Carousel */}
      <div className="bg-gradient-to-br from-[#141E30] via-[#1f2f45] to-[#35577D] py-8 xs:py-12 sm:py-16 overflow-hidden relative">
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_center,_var(--tw-gradient-stops))] from-[#35577D]/20 via-transparent to-transparent pointer-events-none"></div>

        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8 relative z-10">
          <h2 className="text-2xl xs:text-3xl font-bold text-center mb-6 xs:mb-8 sm:mb-12 text-white">Collaboration & Sponsors</h2>

          {/* Scrollable Sponsors Container */}
          <div
            ref={carouselRef}
            className="flex gap-4 xs:gap-6 sm:gap-8 overflow-x-hidden scrollbar-hide"
            style={{ scrollbarWidth: 'none', msOverflowStyle: 'none' }}>

            {/* Render sponsors three times for seamless infinite loop */}
            {[...sponsors, ...sponsors, ...sponsors].map((sponsor, index) => (
              <div key={index} className="flex-shrink-0 w-36 xs:w-40 sm:w-48 text-center transition-opacity duration-700 ease-in-out">
                <div className="relative overflow-hidden backdrop-blur-md bg-white border-[2px] border-[#35577D] p-3 xs:p-4 sm:p-6 rounded-lg xs:rounded-xl shadow-lg h-48 xs:h-56 sm:h-64">
                  <div className="absolute inset-0 bg-gradient-to-br from-[#141E30]/5 to-[#35577D]/5 opacity-30"></div>
                  <div className="relative">
                    <img
                      src={sponsor.logo}
                      alt={sponsor.name}
                      className="h-16 xs:h-20 sm:h-24 w-full object-contain mb-2 xs:mb-3 sm:mb-4"
                    />
                    <p className="text-xs xs:text-sm font-bold text-gray-800 mb-0.5 xs:mb-1">{sponsor.name}</p>
                    <p className="text-xs text-black-400">{sponsor.desc}</p>
                  </div>
                </div>
              </div>
            ))}

          </div>
        </div>
      </div>

      {/* Footer */}
      <footer className="bg-[#141E30] text-white py-4 xs:py-6 sm:py-8">
        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8 relative">
          <div className="flex flex-col md:flex-row justify-center items-center gap-2 xs:gap-3 sm:gap-4">
            {/* Left - NSF Logo */}
            <div className="flex-shrink-0">
              <img
                src="/logos/NSF-NCAR.png"
                alt="NSF-NCAR"
                className="h-16 xs:h-20 sm:h-24 object-contain"
                onError={(e) => e.target.style.display='none'}
              />
            </div>

            {/* Vertical Divider */}
            {/* <div className="hidden md:block w-px h-16 xs:h-20 sm:h-24 bg-gray-500"></div> */}

            {/* Right - NCAR Logo + Text */}
            {/* <div className="flex flex-col items-center md:items-start gap-2 xs:gap-3">
              <img
                src="/logos/NCAR.avif"
                alt="NCAR"
                className="h-12 xs:h-14 sm:h-16 object-contain"
                onError={(e) => e.target.style.display='none'}
              />
              <div className="text-center md:text-left">
                <p className="text-xs xs:text-sm font-semibold tracking-wide">NATIONAL CENTER FOR ATMOSPHERIC</p>
                <p className="text-xs xs:text-sm font-semibold tracking-wide">RESEARCH</p>
              </div>
            </div> */}
          </div>

          {/* Copyright - Bottom Right */}
          <div className="absolute bottom-1 xs:bottom-2 right-2 xs:right-4">
            <p className="text-xs text-gray-400">©Copyright</p>
          </div>
        </div>
      </footer>
    </div>
  )
}

export default AboutPage
