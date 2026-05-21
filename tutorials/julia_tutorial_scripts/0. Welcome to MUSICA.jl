# # Welcome to `Musica`!

# This tutorial will walk you through getting an installation working and running a box model using the `Musica.jl` package in the Julia scientific computing language.

# Note: we'll use `Musica` to refer to the Julia package Musica.jl developed as part of the larger MUlti-Scale Infrastructure for Chemistry and Aerosols (MUSICA) Project. To learn more about the MUSICA Project, see [here](https://www2.acom.ucar.edu/sections/multi-scale-infrastructure-chemistry-modeling-musica).

# ## Installation (optional)

# to install the latest stable version of Julia (as well as the juliaup tool) on macOS or Linux, run the following in your terminal:
# `$curl -fsSL https://install.julialang.org | sh`

# You can then start Julia from the command line by typing `julia`

# Use Julia's package manager to add the Musica.jl package/

# `using Pkg; Pkg.add("Musica")`
# or 
# `]add Musica`

# ## Using `Musica`

# Let's start off with loading `Musica` and getting the versions in Julia.

# Version checks

using Musica
println("using Musica version: ", get_version())

# ## A simple gas-phase system from an existing JSON configuration file

# We willload an existing JSON configuration file that describes a three-species, two-reaction gas-phase system.

# Before we build this model, let's take a look at the config file to see how it's organized.

using JSON
config_path = joinpath(@__DIR__, "../../configs/v1/analytical/config.json")
config = JSON.parsefile(config_path)
println("Contents of the JSON configuration file:")
JSON.print(stdout,config, 2)



# This simple mechanism comprises:

# Three species (A, B, and C) in the gas-phase
# Two Arrhenius reactions
# A -> B
# B -> C
# The format of the Musica configuration is intended to describe the science without introducing details of how Musica software is implemented in code.

# Now, let's evolve this system forward in time using `MICM`: the Model-Independent Chemistry Module, which is the gas-phase solver in `Musica`.
# We'll use the configuration file to create a `MICM` solver and state.

solver = MICM(config_path=config_path)
state = create_state(solver)

# *Why do we need a solver and a state?*

# Conceptually, they fill two distinct roles:
# - The `state`` is a container for time-varying properties for your system of interest (temperature, pressure, species concentrations, etc.)
# - The `solver` is the logic needed to advance a `state` in time. In our case we get the default MICM solver, which is a Rosenbrock 3-stage solver with a predetermined set of solver parameters. We could get a different ODE solver, or set different solver parameters by including additional arguments to the MICM() constructor.

# Practically, having a separate solver and state allows you to create as many state containers as you want. Each state can all be advanced in time using the same solver instance. Each state instance can also be configured to represent multiple grid cells, which you'll read more about in the next tutorial.

# For now, we'll keep things simple with a single one-grid-cell `state`. Let's set the initial conditions and move the chemistry forward!
set_conditions!(state; temperatures=298.15, pressures=101325)
set_concentrations!(state, Dict(
    "A" => 1.0,
    "B" => 0.0,
    "C" => 0.0,
))

# Now that we have our initial environmental conditions and species concentrations set, we can solve and collect the concentrations as they change over time. For now, we'll print them out. Later tutorials have more code to make some pretty figures.

# Let's solve for 10 seconds and see how things change after 10 seconds.
result = solve!(solver, state, 10.0)
get_concentrations(state)

# That's your first `Musica` box model! Read on to learn how to define mechanisms in-code for solving and to do more some more complex tasks.
# ### Turning this script into a notebook

# This script can be turned into a Jupyter notebook using `Literate.jl`:
# `import Literate; Literate.notebook("tutorials/julia_tutorial_scripts/0. Welcome to MUSICA.jl", "tutorials/julia_tutorial_notebooks"; execute=true)`
