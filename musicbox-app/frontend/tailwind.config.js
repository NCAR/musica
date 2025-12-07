/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      screens: {
        'xs': '475px',
        '3xl': '1920px',
      },
      maxWidth: {
        '8xl': '88rem',
        '9xl': '96rem',
      },
      colors: {
        'ncar-blue': '#1e3a8a',
        'ncar-light-blue': '#3b82f6',
      },
      backgroundImage: {
        'hero-pattern': "url('/hero-bg.jpg')",
      },
      animation: {
        'orbit-ring-1': 'orbit-ring 2s infinite linear',
        'orbit-ring-2': 'orbit-ring 2.5s infinite linear',
        'orbit-ring-3': 'orbit-ring 3s infinite linear',
        'orbit-ring-4': 'orbit-ring 3.5s infinite linear',
        'electron-1': 'electron-orbit 2s infinite linear',
        'electron-2': 'electron-orbit 2.5s infinite linear',
        'electron-3': 'electron-orbit 3s infinite linear',
        'electron-4': 'electron-orbit 3.5s infinite linear',
        'nucleus-pulse': 'nucleus-pulse 2s infinite ease-in-out',
        'nucleus-rotate': 'nucleus-rotate 4s infinite linear',
      },
      keyframes: {
        'electron-orbit': {
          '0%': {
            transform: 'rotateZ(0deg) translateX(8rem) rotateZ(0deg) rotateY(-65deg)'
          },
          '100%': {
            transform: 'rotateZ(360deg) translateX(8rem) rotateZ(-360deg) rotateY(-65deg)'
          },
        },
        'orbit-ring': {
          '0%': {
            borderTopColor: 'rgba(255, 255, 255, 0.4)',
            borderRightColor: 'transparent',
            borderBottomColor: 'transparent',
            borderLeftColor: 'transparent'
          },
          '25%': {
            borderTopColor: 'transparent',
            borderRightColor: 'rgba(255, 255, 255, 0.4)',
            borderBottomColor: 'transparent',
            borderLeftColor: 'transparent'
          },
          '50%': {
            borderTopColor: 'transparent',
            borderRightColor: 'transparent',
            borderBottomColor: 'rgba(255, 255, 255, 0.4)',
            borderLeftColor: 'transparent'
          },
          '75%': {
            borderTopColor: 'transparent',
            borderRightColor: 'transparent',
            borderBottomColor: 'transparent',
            borderLeftColor: 'rgba(255, 255, 255, 0.4)'
          },
          '100%': {
            borderTopColor: 'rgba(255, 255, 255, 0.4)',
            borderRightColor: 'transparent',
            borderBottomColor: 'transparent',
            borderLeftColor: 'transparent'
          },
        },
        'nucleus-pulse': {
          '0%': {
            boxShadow: '0 0 20px rgba(251, 191, 36, 0.4)'
          },
          '50%': {
            boxShadow: '0 0 60px rgba(251, 191, 36, 0.8), 0 0 100px rgba(251, 191, 36, 0.4)'
          },
          '100%': {
            boxShadow: '0 0 20px rgba(251, 191, 36, 0.4)'
          },
        },
        'nucleus-rotate': {
          '0%': {
            transform: 'rotate(0deg)'
          },
          '100%': {
            transform: 'rotate(360deg)'
          },
        },
      },
    },
  },
  plugins: [],
}
