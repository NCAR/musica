import { Button } from '../ui/button'
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from '../ui/card'

function HomePage({ onNavigate }) {
  return (
    <div className="min-h-screen">
      {/* Navigation Bar - Sticky */}
      <nav className="bg-gradient-to-br from-white/80 to-[#c5c7cb]/80 backdrop-blur-sm border-b shadow-sm sticky top-0 z-50">
        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between h-14 sm:h-16">
            <div className="flex items-center">
              <h1
                onClick={() => onNavigate('home')}
                className="text-xl sm:text-2xl font-bold text-[#141E30] cursor-pointer hover:opacity-80 transition-opacity truncate"
              >
                MUSICBOX
              </h1>
            </div>
            <div className="flex items-center space-x-2 xs:space-x-4 sm:space-x-6 md:space-x-8">
              <button onClick={() => onNavigate('home')} className="text-[#35577D] font-bold border-b-2 border-[#35577D] text-xs xs:text-sm sm:text-base">Home</button>
              <button onClick={() => onNavigate('about')} className="text-[#141E30] font-bold hover:text-[#35577D] text-xs xs:text-sm sm:text-base">About</button>
              {/* <a href="#model" className="text-gray-700 font-bold hover:text-blue-900">Model</a> */}
              {/* <a href="#data" className="text-gray-700 font-bold hover:text-blue-900">Data</a> */}
              {/* <a href="#documentation" className="text-[#141E30] font-bold hover:text-[#35577D]">Documentation</a> */}
              <button onClick={() => onNavigate('contact')} className="text-[#141E30] font-bold hover:text-[#35577D] text-xs xs:text-sm sm:text-base">Contact</button>
            </div>
          </div>
        </div>
      </nav>

      {/* Hero Section with Video Background + Features Inside */}
      <div className="relative min-h-screen">
        {/* Video Background */}
        <video
          autoPlay
          loop
          muted
          playsInline
          className="absolute inset-0 w-full h-full object-cover"
          poster="/hero-bg.jpg"
        >
          <source src="/videos/hero_movie.mp4" type="video/mp4" />
        </video>

        {/* Dark Overlay */}
        <div className="absolute inset-0 bg-black opacity-50"></div>

        {/* Hero Content Container */}
        <div className="relative z-10 min-h-screen flex flex-col">
          {/* Top Section - Hero Content */}
          <div className="flex-1 flex items-center justify-center px-3 xs:px-4 sm:px-6 md:px-8 pb-16 xs:pb-20 sm:pb-24">
            <div className="text-center text-white max-w-4xl mx-auto">
              <h1 className="text-2xl xs:text-3xl sm:text-4xl md:text-5xl lg:text-6xl xl:text-7xl font-bold mb-4 xs:mb-5 sm:mb-6 leading-tight">
                Explore Atmospheric Chemistry
                <br />
                Through MusicBox
              </h1>
              <p className="text-sm xs:text-base sm:text-lg md:text-xl lg:text-2xl mb-6 xs:mb-8 sm:mb-10 text-gray-200 px-2">
                A powerful box model for simulating atmospheric chemistry reactions
              </p>
              <Button
                size="lg"
                className="group relative overflow-hidden backdrop-blur-2xl bg-white/20 border-2 border-white/60 text-white hover:bg-white/30 hover:scale-105 px-4 xs:px-6 sm:px-8 md:px-10 py-3 xs:py-4 sm:py-5 md:py-7 text-sm xs:text-base sm:text-lg md:text-xl font-bold shadow-2xl hover:shadow-white/50 transition-all duration-500"
                onClick={() => onNavigate('app')}
              >
                <span className="relative z-10 drop-shadow-lg rounded-lg">Launch MusicBox</span>
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

          {/* Bottom Section - Feature Cards with Glass Effect */}
          <div className="pb-8 xs:pb-12 sm:pb-16 px-2 xs:px-3 sm:px-4 md:px-6 lg:px-8">
            <div className="max-w-7xl mx-auto">
              <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 gap-4 xs:gap-6 sm:gap-8">
                {/* Card 1 - Interactive Modeling Tools with 2 Icons */}
                <div className="group h-full">
                  <div className="relative h-full overflow-hidden rounded-xl xs:rounded-2xl backdrop-blur-xl bg-white/10 border border-white/20 shadow-2xl">
                    <div className="absolute inset-0 bg-gradient-to-br from-white/20 to-transparent opacity-50"></div>
                    <div className="relative p-4 xs:p-6 sm:p-8 text-center flex flex-col h-full">
                      {/* Dual Icon Container with Vertical Bar */}
                      <div className="mx-auto mb-3 xs:mb-4 flex items-center justify-center gap-2 xs:gap-3 sm:gap-4">
                        <div className="w-12 h-12 xs:w-16 xs:h-16 sm:w-20 sm:h-20 rounded-lg backdrop-blur-sm bg-gradient-to-br bg-white/5 border border-white/50 flex items-center justify-center shadow-lg p-2 xs:p-2.5 sm:p-3">
                          <img src="/icons/analysis.png" alt="Analysis" className="w-full h-full object-contain" />
                        </div>
                        {/* Vertical Bar */}
                        <div className="w-0.5 xs:w-1 h-12 xs:h-16 sm:h-20 bg-white/30 rounded-full"></div>
                        <div className="w-12 h-12 xs:w-16 xs:h-16 sm:w-20 sm:h-20 rounded-lg backdrop-blur-sm bg-white/5 border border-white/40 flex items-center justify-center shadow-lg p-2 xs:p-2.5 sm:p-3">
                          <img src="/icons/project-management.png" alt="Project Management" className="w-full h-full object-contain" />
                        </div>
                      </div>
                      <h3 className="text-base xs:text-lg sm:text-xl md:text-2xl font-bold mb-2 xs:mb-3 mt-1 xs:mt-2 text-white">Interactive Modeling Tools</h3>
                      <p className="text-xs xs:text-sm sm:text-base leading-relaxed text-white/90 mb-3 xs:mb-4 flex-grow">
                        Simulate chamber experiments, recreate field observations, or evaluate new chemistry systems
                        with our intuitive web-based interface
                      </p>
                    </div>
                  </div>
                </div>

                {/* Card 2 - Comprehensive Data Archives */}
                <div className="group h-full">
                  <div className="relative h-full overflow-hidden rounded-xl xs:rounded-2xl backdrop-blur-xl bg-white/10 border border-white/20 shadow-2xl">
                    <div className="absolute inset-0 bg-gradient-to-br from-white/20 to-transparent opacity-50"></div>
                    <div className="relative p-4 xs:p-6 sm:p-8 text-center flex flex-col h-full">
                      <div className="mx-auto mb-4 xs:mb-5 sm:mb-6 w-12 h-12 xs:w-16 xs:h-16 sm:w-20 sm:h-20 rounded-lg backdrop-blur-sm bg-gradient-to-br bg-white/5 border border-white/50 flex items-center justify-center shadow-lg p-2 xs:p-2.5 sm:p-3">
                        <img src="/icons/database-storage.png" alt="Database" className="w-full h-full object-contain" />
                      </div>
                      <h3 className="text-base xs:text-lg sm:text-xl md:text-2xl font-bold mb-2 xs:mb-3 text-white">Comprehensive Data Archives</h3>
                      <p className="text-xs xs:text-sm sm:text-base leading-relaxed text-white/90 mb-3 xs:mb-4 flex-grow">
                        Access NCAR-validated mechanisms including TS1 (209 species), Chapman, and custom
                        configurations for your research needs
                      </p>
                    </div>
                  </div>
                </div>

                {/* Card 3 - Collaborative Research Environment */}
                <div className="group h-full">
                  <div className="relative h-full overflow-hidden rounded-xl xs:rounded-2xl backdrop-blur-xl bg-white/10 border border-white/20 shadow-2xl">
                    <div className="absolute inset-0 bg-gradient-to-br from-white/20 to-transparent opacity-50"></div>
                    <div className="relative p-4 xs:p-6 sm:p-8 text-center flex flex-col h-full">
                      <div className="mx-auto mb-4 xs:mb-5 sm:mb-6 w-12 h-12 xs:w-16 xs:h-16 sm:w-20 sm:h-20 rounded-lg backdrop-blur-sm bg-gradient-to-br bg-white/5 border border-white/50 flex items-center justify-center shadow-lg p-2 xs:p-2.5 sm:p-3">
                        <img src="/icons/ai-research.png" alt="AI Research" className="w-full h-full object-contain" />
                      </div>
                      <h3 className="text-base xs:text-lg sm:text-xl md:text-2xl font-bold mb-2 xs:mb-3 text-white">Collaborative Research Environment</h3>
                      <p className="text-xs xs:text-sm sm:text-base leading-relaxed text-white/90 mb-3 xs:mb-4 flex-grow">
                        Share your chemical systems with the community for use in participating models
                        and advance atmospheric science together
                      </p>
                    </div>
                  </div>
                </div>
              </div>
            </div>
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

            {/* Vertical Divider
            <div className="hidden md:block w-px h-16 xs:h-20 sm:h-24 bg-gray-500"></div> */}

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

export default HomePage
