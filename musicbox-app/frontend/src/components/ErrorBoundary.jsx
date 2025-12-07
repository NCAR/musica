import React from 'react'
import { Card, CardContent } from './ui/card'
import { AlertCircle } from 'lucide-react'

/**
 * ErrorBoundary Component
 * Catches React errors in child components and displays a fallback UI
 */
class ErrorBoundary extends React.Component {
  constructor(props) {
    super(props)
    this.state = { hasError: false, error: null, errorInfo: null }
  }

  static getDerivedStateFromError(error) {
    // Update state so the next render will show the fallback UI
    return { hasError: true }
  }

  componentDidCatch(error, errorInfo) {
    // Log the error to console
    console.error('ErrorBoundary caught an error:', error, errorInfo)
    this.state = { ...this.state, error, errorInfo }
  }

  render() {
    if (this.state.hasError) {
      return (
        <Card className="border-2 border-red-500 bg-red-50">
          <CardContent className="pt-6">
            <div className="text-center">
              <div className="flex justify-center mb-3">
                <AlertCircle className="w-16 h-16 text-red-600" />
              </div>
              <h3 className="text-lg font-bold text-red-900 mb-2">
                Oops! Something went wrong
              </h3>
              <p className="text-sm text-red-700 mb-4">
                {this.state.error?.message || 'An error occurred while rendering this component'}
              </p>
              <button
                onClick={() => this.setState({ hasError: false, error: null, errorInfo: null })}
                className="px-4 py-2 bg-red-600 text-white rounded-lg hover:bg-red-700 transition-colors"
              >
                Try Again
              </button>
              {process.env.NODE_ENV === 'development' && this.state.errorInfo && (
                <details className="mt-4 text-left">
                  <summary className="cursor-pointer text-sm font-semibold text-red-800">
                    Error Details (Development Only)
                  </summary>
                  <pre className="mt-2 text-xs bg-red-100 p-3 rounded overflow-auto max-h-64">
                    {this.state.error?.stack}
                  </pre>
                </details>
              )}
            </div>
          </CardContent>
        </Card>
      )
    }

    return this.props.children
  }
}

export default ErrorBoundary
