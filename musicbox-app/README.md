# Project: TAMUG Capstone 2025 - MusicBox Interactive
- Author: Jason Nguyen, Miles Ryders, Brayton Lee

## Musicbox Interactive

 a web app for running atmospheric chemistry simulations. It's basically a gui wrapper around the MUSICA/MICM chemistry solver. We built this as part of my capstone project at TAMU.

Users can:
- create custom chemical mechanisms
- run simulations with different conditions
- visualize concentration profiles over time
- load/save configurations as json files

## prerequisites

before you start, make sure you got these installed:

- **node.js** (v18 or newer) - for running the frontend and backend
- **npm** - comes with node, used for package management
- **cmake** (3.21+) - required to build the C++ addon
- **c++ compiler** - g++, clang, or MSVC (comes with Xcode on macOS)

## installation

**IMPORTANT:** You must build the C++ addon BEFORE installing the musicbox-app packages!

### quick start (tl;dr)

```
# clone repo and enter directory
git clone https://github.com/NCAR/musica.git
cd musica

# build the C++ addon (this is the critical step!)
npm install
npm run build

# install web app dependencies
cd musicbox-app
npm run install:all

# run the servers (use 2 terminals)
npm run server    # terminal 1 - backend on :3001
npm run dev       # terminal 2 - frontend on :5173
```

### step 1: clone the repository

```
git clone https://github.com/NCAR/musica.git
cd musica
```

### step 2: install root dependencies

from the root `musica/` directory:
```
npm install
```

this installs cmake-js and node-addon-api needed for building the C++ addon.

### step 3: build the C++ addon

still in the root `musica/` directory:
```
npm run build
```

this compiles the MUSICA/MICM chemistry solver into a native Node.js addon. takes about 2-3 minutes.

you should see output ending with:
```
[100%] Built target musica-addon
```

the compiled addon will be at: `build/Release/musica-addon.node`

### step 4: install musicbox-app dependencies

now go into the `musicbox-app/` directory:
```
cd musicbox-app
npm run install:all
```

this installs packages for root, frontend, and backend. it's basically running:
```
npm install               # root packages
cd frontend && npm install
cd backend && npm install
```

if you see warnings about peer dependencies or engine versions, usually safe to ignore. we're using react 19 which is pretty new.

## running the app

you need two terminals running at the same time:

**terminal 1 - backend server:**
```
cd musicbox-app
npm run server
```

this starts the express server on `http://localhost:3001`

you should see:
```
MusicBox API Server running on http://localhost:3001
   Health check: http://localhost:3001/api/health
   ...
```

**terminal 2 - frontend dev server:**
```
cd musicbox-app
npm run dev
```

this starts vite on `http://localhost:5173`

open `http://localhost:5173` in your browser and you're good to go!

## Common issues

# "cannot find module 'musica-addon.node'"

the c++ addon wasn't built. go back to the root `musica/` directory and run:
```
npm install
npm run build
```

# backend crashes with segfault

this happened to me when using v0 CAMP configs. Only use v1 format configs (the ones in `configs/v1/`). v0 format has known issues.

# port already in use

if 3001 or 5173 is taken:

kill the process:
```
# find what's using the port
lsof -i :3001
kill -9 <PID>
```

or change the port in backend/index.js or vite config

# "module not found" errors in frontend

delete node_modules and reinstall:
```
cd frontend
rm -rf node_modules
npm install
```

## example configs

load one of these to test quickly:
- **chapman** - simple stratospheric oxygen chemistry (5 species, 6 reactions)
- **ts1** - big tropospheric mechanism (209 species, 512 reactions)
- **full_configuration** - tests all reaction types

find them in `configs/v1/`

each mechanism folder has two different json files:

```
configs/v1/chapman/
─ config.json          # mechanism only (species + reactions)
─ example.json         # I have created complete example (mechanism + conditions + params)
```

**config.json** - this is what the chemistry solver actually uses
- just has the mechanism definition
- species, reactions, phases
- no initial concentrations or simulation settings
- backend loads this automatically when you select a predefined mechanism

**example.json** - this is what you load from the UI
- full configuration file
- has mechanism + initial conditions + simulation parameters
- use this when you want a ready-to-run example
- click "Load Example" in the dashboard to use these

so basically:
- backend uses `config.json` to run the chemistry
- frontend loads `example.json` when you want a complete setup

## tech stack

for reference, here's what we're using:

**frontend:**
- react 19 with vite
- redux toolkit for state management
- tailwindcss + shadcn/ui components
- recharts for plotting
- axios for api calls

**backend:**
- express.js
- node.js n-api for c++ bindings
- musica/micm chemistry solver

**chemistry:**
- musica c++ library
- micm solver (rosenbrock)
- json mechanism configs

# React + Vite

This template provides a minimal setup to get React working in Vite with HMR and some ESLint rules.

Currently, two official plugins are available:

- [@vitejs/plugin-react](https://github.com/vitejs/vite-plugin-react/blob/main/packages/plugin-react) uses [Babel](https://babeljs.io/) (or [oxc](https://oxc.rs) when used in [rolldown-vite](https://vite.dev/guide/rolldown)) for Fast Refresh
- [@vitejs/plugin-react-swc](https://github.com/vitejs/vite-plugin-react/blob/main/packages/plugin-react-swc) uses [SWC](https://swc.rs/) for Fast Refresh

## React Compiler

The React Compiler is not enabled on this template because of its impact on dev & build performances. To add it, see [this documentation](https://react.dev/learn/react-compiler/installation).

## Expanding the ESLint configuration

If you are developing a production application, we recommend using TypeScript with type-aware lint rules enabled. Check out the [TS template](https://github.com/vitejs/vite/tree/main/packages/create-vite/template-react-ts) for information on how to integrate TypeScript and [`typescript-eslint`](https://typescript-eslint.io) in your project.
