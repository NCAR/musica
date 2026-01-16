from musica.micm import MICM, State, SolverType, SolverResult, SolverState
import musica.mechanism_configuration as mc
import matplotlib.pyplot as plt
from matplotlib import animation
from matplotlib.animation import FFMpegWriter, PillowWriter
import argparse
import os

def _to_scalar(v):
    try:
        return float(v)
    except Exception:
        # numpy scalar
        try:
            if hasattr(v, 'item'):
                return float(v.item())
        except Exception:
            pass
        # single-element sequence
        try:
            if hasattr(v, '__len__') and len(v) == 1:
                return float(v[0])
        except Exception:
            pass
        raise TypeError(f"Cannot convert value to scalar: {type(v)}")

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


def main(output='lorenz.mp4', fps=30):
    mechanism = create_lorenz_mechanism()

    # Initialize MICM
    micm = MICM(mechanism=mechanism, solver_type=SolverType.rosenbrock_standard_order)
    state = micm.create_state(1)

    # Rate parameters
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
    state.set_user_defined_rate_parameters(rate_params)
    state.set_concentrations({
            "X": 1.0,
            "Y": 28.0,
            "Z": 1.0
    })

    nsteps = 2000
    burnout = 200

    times = []
    Xs = []
    Ys = []
    Zs = []
    cumulative_time = 0.0
    time_step = 0.01

    for step in range(nsteps):
        actual_solve = 0
        while actual_solve < time_step:
            result = micm.solve(state, time_step=time_step)
            actual_solve += result.stats.final_time
            cumulative_time += result.stats.final_time

        if step < burnout:
            continue

        concs = state.get_concentrations()
        times.append(cumulative_time)
        Xs.append(concs["X"])
        Ys.append(concs["Y"])
        Zs.append(concs["Z"])

    print("Simulation complete.")

    def create_animation(Xs, Ys, Zs, outpath='lorenz.mp4', fps=30):
        # Ensure values are scalars (arrays of length 1 from the state)
        Xs = [_to_scalar(x) for x in Xs]
        Ys = [_to_scalar(y) for y in Ys]
        Zs = [_to_scalar(z) for z in Zs]

        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')

        ax.set_xlabel('X Concentration')
        ax.set_ylabel('Y Concentration')
        ax.set_zlabel('Z Concentration')
        ax.set_title('Lorenz Attractor from Chemical Reaction Network')

        xmin, xmax = min(Xs), max(Xs)
        ymin, ymax = min(Ys), max(Ys)
        zmin, zmax = min(Zs), max(Zs)
        ax.set_xlim(xmin, xmax)
        ax.set_ylim(ymin, ymax)
        ax.set_zlim(zmin, zmax)

        line, = ax.plot([], [], [], lw=1)
        point, = ax.plot([], [], [], 'o', color='red', markersize=4)

        def init():
            line.set_data([], [])
            line.set_3d_properties([])
            point.set_data([], [])
            point.set_3d_properties([])
            return line, point

        def update(i):
            line.set_data(Xs[:i], Ys[:i])
            line.set_3d_properties(Zs[:i])
            if i > 0:
                point.set_data([Xs[i-1]], [Ys[i-1]])
                point.set_3d_properties([Zs[i-1]])
            return line, point

        frames = len(Xs)
        interval = 1000.0 / fps

        anim = animation.FuncAnimation(
                fig, update, init_func=init, frames=frames, interval=interval, blit=True)

        outdir = os.path.dirname(outpath) or '.'
        os.makedirs(outdir, exist_ok=True)

        try:
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
    args = parser.parse_args()

    # Rerun main with requested output and fps
    main(output=args.output, fps=args.fps)