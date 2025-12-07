import { Button } from '../ui/button'
import { Card, CardContent } from '../ui/card'

function ContactPage({ onNavigate }) {
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
              <button onClick={() => onNavigate('about')} className="text-[#141E30] hover:text-[#35577D] font-bold text-xs xs:text-sm sm:text-base">About</button>
              {/* <a href="#model" className="text-gray-700 hover:text-blue-900">Model</a> */}
              {/* <a href="#data" className="text-gray-700 hover:text-blue-900">Data</a> */}
              {/* <a href="#documentation" className="text-[#141E30] font-bold hover:text-[#35577D]">Documentation</a> */}
              <button onClick={() => onNavigate('contact')} className="text-[#35577D] font-bold border-b-2 border-[#35577D] text-xs xs:text-sm sm:text-base">Contact</button>
            </div>
          </div>
        </div>
      </nav>

      {/* Contact Hero Section */}
      <div className="bg-gradient-to-r from-[#1f2839] to-[#28364c] text-white py-8 xs:py-12 sm:py-16">
        <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8 text-center">
          <h1 className="text-2xl xs:text-3xl sm:text-4xl md:text-5xl font-bold mb-2 xs:mb-3 sm:mb-4">Contact Us</h1>
          <p className="text-sm xs:text-base sm:text-lg md:text-xl text-blue-100 max-w-3xl mx-auto px-2">
            Have questions or feedback? We'd love to hear from you
          </p>
        </div>
      </div>

      {/* Contact Form Section */}
      <div className="max-w-7xl mx-auto px-2 xs:px-4 sm:px-6 lg:px-8 py-8 xs:py-12 sm:py-16">
        <div className="max-w-2xl mx-auto">
          <Card className="shadow-xl">
            <CardContent className="p-4 xs:p-6 sm:p-8">
              <form className="space-y-4 xs:space-y-5 sm:space-y-6">
                <div>
                  <label className="block text-xs xs:text-sm font-semibold text-gray-700 mb-1 xs:mb-2">Name</label>
                  <input
                    type="text"
                    className="w-full px-3 xs:px-4 py-2 xs:py-2.5 sm:py-3 text-sm xs:text-base border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent transition-all"
                    placeholder="Your name"
                  />
                </div>
                <div>
                  <label className="block text-xs xs:text-sm font-semibold text-gray-700 mb-1 xs:mb-2">Email</label>
                  <input
                    type="email"
                    className="w-full px-3 xs:px-4 py-2 xs:py-2.5 sm:py-3 text-sm xs:text-base border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent transition-all"
                    placeholder="your.email@example.com"
                  />
                </div>
                <div>
                  <label className="block text-xs xs:text-sm font-semibold text-gray-700 mb-1 xs:mb-2">Subject</label>
                  <input
                    type="text"
                    className="w-full px-3 xs:px-4 py-2 xs:py-2.5 sm:py-3 text-sm xs:text-base border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent transition-all"
                    placeholder="What is this about?"
                  />
                </div>
                <div>
                  <label className="block text-xs xs:text-sm font-semibold text-gray-700 mb-1 xs:mb-2">Message</label>
                  <textarea
                    rows={6}
                    className="w-full px-3 xs:px-4 py-2 xs:py-2.5 sm:py-3 text-sm xs:text-base border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-600 focus:border-transparent transition-all resize-none"
                    placeholder="Your message..."
                  ></textarea>
                </div>
                <Button className="w-full bg-[#141E30] hover:bg-[#35577D] text-white py-2 xs:py-2.5 sm:py-3 text-sm xs:text-base sm:text-lg font-semibold">
                  Send Message
                </Button>
              </form>
            </CardContent>
          </Card>

          {/* Additional Contact Info */}
          <div className="mt-6 xs:mt-8 sm:mt-12 grid grid-cols-1 xs:grid-cols-2 gap-4 xs:gap-6 sm:gap-8">
            <div className="bg-blue-50 p-4 xs:p-5 sm:p-6 rounded-lg">
              <h3 className="text-base xs:text-lg font-bold text-[#2c3a50] mb-1 xs:mb-2">Email</h3>
              <p className="text-gray-700 text-xs xs:text-sm sm:text-base">support@musicbox.ncar.edu</p>
            </div>
            <div className="bg-blue-50 p-4 xs:p-5 sm:p-6 rounded-lg">
              <h3 className="text-base xs:text-lg font-bold text-[#2c3a50] mb-1 xs:mb-2">Location</h3>
              <p className="text-gray-700 text-xs xs:text-sm sm:text-base">National Center for Atmospheric Research</p>
              <p className="text-gray-700 text-xs xs:text-sm sm:text-base">Boulder, Colorado, USA</p>
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

          
        </div>
        {/* Copyright - Bottom Right */}
        <div className="absolute bottom-1 xs:bottom-2 right-2 xs:right-4">
          <p className="text-[10px] sm:text-xs text-gray-400">©Copyright</p>
        </div>
      </footer>
    </div>
  )
}

export default ContactPage
