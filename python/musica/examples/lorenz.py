from musica.micm import MICM, SolverType
import musica.mechanism_configuration as mc
import matplotlib.pyplot as plt
from matplotlib import animation
from matplotlib.animation import FFMpegWriter, PillowWriter
import argparse
import os
import numpy as np


def full_step(micm, state, time_step):
    """
    Advance the MICM state by a full time step, retrying if necessary.

    Parameters
    ----------
    micm : MICM
        The MICM solver instance.
    state : MICM.State
        The current state of the system.
    time_step : float
        The time step to advance.
    """

    elapsed_time = 0
    while elapsed_time < time_step:
        result = micm.solve(state, time_step=time_step)
        elapsed_time += result.stats.final_time


def create_lorenz_mechanism():
    """
    Create a Lorenz mechanism

    from https://doi.org/10.1007/s11071-025-11622-1
    """
    # Species
    X = mc.Species(name="X")
    Y = mc.Species(name="Y")
    Z = mc.Species(name="Z")

    # Gas phase
    gas = mc.Phase(name="gas", species=[X, Y, Z])

    # User-defined reactions
    reactions = [
        mc.UserDefined(
            name="X_decay",
            gas_phase=gas,
            reactants=[X],
            products=[]
        ),
        mc.UserDefined(
            name="Y_to_XY",
            gas_phase=gas,
            reactants=[Y],
            products=[X, Y]
        ),
        mc.UserDefined(
            name="Y_source",
            gas_phase=gas,
            reactants=[],
            products=[Y]
        ),
        mc.UserDefined(
            name="Y_sink",
            gas_phase=gas,
            reactants=[Y],
            products=[]
        ),
        mc.UserDefined(
            name="XY_to_X2Y",
            gas_phase=gas,
            reactants=[X, Y],
            products=[X, Y, Y]
        ),
        mc.UserDefined(
            name="YZ_to_2Y",
            gas_phase=gas,
            reactants=[Y, Z],
            products=[Y, Y]
        ),
        mc.UserDefined(
            name="XYZ_to_X2Z",
            gas_phase=gas,
            reactants=[X, Y, Z],
            products=[X, Z, Z]
        ),
        mc.UserDefined(
            name="Z_source",
            gas_phase=gas,
            reactants=[],
            products=[Z]
        ),
        mc.UserDefined(
            name="Z_autocatalytic",
            gas_phase=gas,
            reactants=[Z],
            products=[Z, Z]
        ),
        mc.UserDefined(
            name="XZ_quench",
            gas_phase=gas,
            reactants=[X, Z],
            products=[X]
        )
    ]

    # Mechanism
    mechanism = mc.Mechanism(
        name="Lorenz Polynomial CRN",
        species=[X, Y, Z],
        phases=[gas],
        reactions=reactions
    )
    return mechanism


def main(output='lorenz.mp4', fps=30, n=2):
    """
    Run the Lorenz polynomial chemical reaction network simulation and save an animation.
    Parameters
    ----------
    output : str, optional
        Path to the output animation file (e.g., MP4 or GIF) to be written. Defaults
        to ``"lorenz.mp4"``.
    fps : int, optional
        Frames per second for the generated animation. Defaults to 30.
    n : int, optional
        The number of grid cells to simulate the attractor in
    Notes
    -----
    This function constructs the Lorenz mechanism, initializes the MICM solver, sets
    rate parameters and initial concentrations, advances the system for a fixed number
    of time steps, and produces a time-evolving visualization of the state variables.
    """
    mechanism = create_lorenz_mechanism()

    # Initialize MICM with `n` grid cells (one State containing N cells)
    micm = MICM(mechanism=mechanism, solver_type=SolverType.rosenbrock_standard_order)
    state = micm.create_state(n)

    # Rate parameters (broadcast to each grid cell)
    mu = 1.0 / 100
    rate_params = {
        "USER.X_decay": 10,
        "USER.Y_to_XY": 10,
        "USER.Y_source": 1 / mu,
        "USER.Y_sink": 1 / mu + 29,
        "USER.XY_to_X2Y": 1 + mu * 28,
        "USER.YZ_to_2Y": 1,
        "USER.XYZ_to_X2Z": mu,
        "USER.Z_source": 8 / (3 * mu),
        "USER.Z_autocatalytic": 1 / mu - 8 / 3,
        "USER.XZ_quench": 1
    }
    # Broadcast scalar rate parameters to lists for each grid cell
    rate_params_broadcast = {k: [v] * n for k, v in rate_params.items()}
    state.set_user_defined_rate_parameters(rate_params_broadcast)

    # Base initial concentrations for each grid cell; apply small perturbations
    base_init = {"X": 1.0, "Y": 28.0, "Z": 1.0}
    delta = 1e-1
    X_init = [base_init["X"] + i * delta for i in range(n)]
    Y_init = [base_init["Y"] + i * delta for i in range(n)]
    Z_init = [base_init["Z"] for _ in range(n)]
    state.set_concentrations({"X": X_init, "Y": Y_init, "Z": Z_init})

    nsteps = 2000
    burnout = 200

    # store trajectories per grid cell: lists of length n containing lists of values
    Xs = [[] for _ in range(n)]
    Ys = [[] for _ in range(n)]
    Zs = [[] for _ in range(n)]
    time_step = 0.01
    for step in range(nsteps):
        # advance the multi-cell state by one time_step
        full_step(micm, state, time_step)

        if step < burnout:
            continue

        concs = state.get_concentrations()
        # Ensure arrays and flatten
        Xvals = np.ravel(concs["X"])
        Yvals = np.ravel(concs["Y"])
        Zvals = np.ravel(concs["Z"])

        # Append each grid cell's current concentration
        for i in range(n):
            Xs[i].append(float(Xvals[i]))
            Ys[i].append(float(Yvals[i]))
            Zs[i].append(float(Zvals[i]))

    print("Simulation complete.")

    def create_animation(Xs, Ys, Zs, outpath='lorenz.mp4', fps=30):
        # Xs, Ys, Zs are lists of length n each containing a time-series list
        Ncells = len(Xs)
        if Ncells == 0:
            print("No trajectory data to animate.")
            return

        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')

        ax.set_xlabel('X Concentration')
        ax.set_ylabel('Y Concentration')
        ax.set_zlabel('Z Concentration')
        ax.set_title('Lorenz Attractor from Chemical Reaction Network')

        xmin = min(min(xs) for xs in Xs)
        xmax = max(max(xs) for xs in Xs)
        ymin = min(min(ys) for ys in Ys)
        ymax = max(max(ys) for ys in Ys)
        zmin = min(min(zs) for zs in Zs)
        zmax = max(max(zs) for zs in Zs)
        ax.set_xlim(xmin, xmax)
        ax.set_ylim(ymin, ymax)
        ax.set_zlim(zmin, zmax)
        # create one line+point per grid cell
        cmap = plt.get_cmap('tab10')
        colors = [cmap(i % 10) for i in range(Ncells)]

        lines = []
        points = []
        for idx in range(Ncells):
            ln, = ax.plot([], [], [], lw=1, color=colors[idx], alpha=0.5, label=f'cell {idx}')
            pt, = ax.plot([], [], [], 'o', color=colors[idx], markersize=3)
            lines.append(ln)
            points.append(pt)

        def init():
            artists = []
            for ln, pt in zip(lines, points):
                ln.set_data([], [])
                ln.set_3d_properties([])
                pt.set_data([], [])
                pt.set_3d_properties([])
                artists.extend([ln, pt])
            return artists

        def update(i):
            artists = []
            for idx in range(Ncells):
                xs = Xs[idx]
                ys = Ys[idx]
                zs = Zs[idx]
                # clamp i for safety
                j = min(i, len(xs) - 1)
                lines[idx].set_data(xs[:j], ys[:j])
                lines[idx].set_3d_properties(zs[:j])
                if j > 0:
                    points[idx].set_data([xs[j - 1]], [ys[j - 1]])
                    points[idx].set_3d_properties([zs[j - 1]])
                artists.extend([lines[idx], points[idx]])
            return artists

        frames = len(Xs[0])
        interval = 1000.0 / fps

        anim = animation.FuncAnimation(
            fig, update, init_func=init, frames=frames, interval=interval, blit=True)

        outdir = os.path.dirname(outpath) or '.'
        os.makedirs(outdir, exist_ok=True)

        try:
            ax.legend()
            writer = FFMpegWriter(fps=fps)
            anim.save(outpath, writer=writer)
            print(f"Saved animation to {outpath}")
        except Exception:
            try:
                gif_path = os.path.splitext(outpath)[0] + '.gif'
                writer = PillowWriter(fps=fps)
                anim.save(gif_path, writer=writer)
                print(f"FFmpeg unavailable; saved GIF to {gif_path}")
            except Exception as e:
                print("Failed to save animation:", e)

    # Save animation with defaults; CLI can override via args below
    create_animation(Xs, Ys, Zs, outpath=output, fps=fps)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run Lorenz CRN simulation and create animation')
    parser.add_argument('--output', '-o', default='lorenz.mp4', help='Output movie file (mp4 preferred)')
    parser.add_argument('--fps', type=int, default=30, help='Frames per second for the movie')
    parser.add_argument('--n', type=int, default=2, help='Number of grid cells / trajectories')
    args = parser.parse_args()

    # Rerun main with requested output and fps
    main(output=args.output, fps=args.fps, n=args.n)
