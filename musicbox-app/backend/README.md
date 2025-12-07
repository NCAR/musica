# MusicBox API Server

Express.js REST API server for running MUSICA atmospheric chemistry simulations.

## Prerequisites

1. Build the MUSICA Node.js addon:
   bash
   cd /Users/jason.blvck/Documents/Courses/Fall25/TAMU_project/musica
   npm run build
   

2. Ensure the build was successful and `build/Release/musica.node` exists.

## Running the Server

From the musicbox-app directory:

bash
# Production mode
npm run server

# Development mode (with auto-reload)
npm run server:dev


The server will start on `http://localhost:3001`

## API Endpoints

## Health Check
GET /api/health


# Mechanisms

**List all mechanisms:**

GET /api/mechanisms


**Get specific mechanism:**

GET /api/mechanisms/:id


**Get mechanism species:**

GET /api/mechanisms/:id/species


## Simulation

**Run simulation:**

POST /api/simulation/run
Content-Type: application/json

{
  "mechanism": "chapman",
  "temperature": 272.5,
  "pressure": 101253.3,
  "timeStep": 200,
  "duration": 3600,
  "initialConcentrations": {
    "O2": 0.75,
    "O": 0.0,
    "O1D": 0.0,
    "O3": 0.0000081
  },
  "rateConstants": {
    "PHOTO.jO2": 2.42e-17,
    "PHOTO.jO3->O": 1.15e-5,
    "PHOTO.jO3->O1D": 6.61e-9
  },
  "outputFrequency": 10
}


**Get simulation results:**

GET /api/simulation/:id


**List all simulations:**

GET /api/simulation


**Delete simulation:**

DELETE /api/simulation/:id


## Example Usage

bash
# Test server health
curl http://localhost:3001/api/health

# List mechanisms
curl http://localhost:3001/api/mechanisms

# Run Chapman mechanism simulation
curl -X POST http://localhost:3001/api/simulation/run \
  -H "Content-Type: application/json" \
  -d '{
    "mechanism": "chapman",
    "temperature": 272.5,
    "pressure": 101253.3,
    "timeStep": 200,
    "duration": 3600,
    "initialConcentrations": {
      "O2": 0.75,
      "O": 0.0,
      "O1D": 0.0,
      "O3": 0.0000081
    },
    "rateConstants": {
      "PHOTO.jO2": 2.42e-17,
      "PHOTO.jO3->O": 1.15e-5,
      "PHOTO.jO3->O1D": 6.61e-9
    }
  }'


## Architecture

**index.js**: Main Express server setup
**routes/mechanisms.js**: Mechanism listing and details
**routes/simulation.js**: Simulation execution using MUSICA addon

## Notes

The server uses in-memory storage for simulation results
For production, consider using a database for persistence
CORS is enabled for development (restrict for production)
The server requires the MUSICA C++ addon to be built
