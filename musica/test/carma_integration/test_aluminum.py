import pytest
import musica
import numpy as np

def aluminum_config():
        """Create parameters for aluminum test configuration."""
        # Set up a wavelength grid
        wavelength_bins = [
            musica.carma.CARMAWavelengthBin(
                center=0.55e-6, width=0.01e-6, do_emission=True),
            musica.carma.CARMAWavelengthBin(
                center=0.65e-6, width=0.01e-6, do_emission=True),
            musica.carma.CARMAWavelengthBin(
                center=0.75e-6, width=0.01e-6, do_emission=True),
            musica.carma.CARMAWavelengthBin(
                center=0.85e-6, width=0.01e-6, do_emission=True),
            musica.carma.CARMAWavelengthBin(
                center=0.95e-6, width=0.01e-6, do_emission=True)
        ]

        # Create aluminum group
        group = musica.carma.CARMAGroupConfig(
            name="aluminum",
            shortname="PRALUM",
            rmin=21.5e-8,
            rmrat=2.0,
            ishape=musica.carma.ParticleShape.SPHERE,
            eshape=1.0,
            mie_calculation_algorithm=musica.carma.MieCalculationAlgorithm.TOON_1981,
            is_ice=False,
            is_fractal=True,
            do_wetdep=False,
            do_drydep=True,
            do_vtran=True,
            solfac=0.0,
            scavcoef=0.0,
            rmon=21.5e-8,
            df=[1.6] * 5,  # 5 bins with fractal dimension 1.6
            falpha=1.0
        )

        # Create aluminum element
        element = musica.carma.CARMAElementConfig(
            igroup=1,
            isolute=0,
            name="Aluminum",
            shortname="ALUM",
            itype=musica.carma.ParticleType.INVOLATILE,
            icomposition=musica.carma.ParticleComposition.ALUMINUM,
            is_shell=True,
            rho=2700.0,  # kg/m3
            arat=[1.0] * 5,  # 5 bins with area ratio 1.0
            kappa=0.0,
        )

        # Create coagulation
        coagulation = musica.carma.CARMACoagulationConfig(
            igroup1=1,
            igroup2=1,
            igroup3=1,
            algorithm=musica.carma.ParticleCollectionAlgorithm.FUCHS)

        params = musica.carma.CARMAParameters(
            nz=1,
            nbin=5,
            deltaz=1000.0,
            zmin=16500.0,
            wavelength_bins=wavelength_bins,
            groups=[group],
            elements=[element],
            coagulations=[coagulation]
        )

        return params

def test_aluminum():

    # Create CARMA solver instance with aluminum configuration
    test_params = aluminum_config()
    carma = musica.CARMA(test_params)
    assert carma is not None
    assert isinstance(carma, musica.CARMA)

    # Create a state loaded with US Standard Atmosphere 1976 data
    state = carma.create_state(
        time_step=1800.0,  # 30 minutes
        longitude=-105.0,
        latitude=0.0)
        
    # Set up time steps
    FIVE_DAYS_IN_SECONDS = 432000
    num_timesteps = int(FIVE_DAYS_IN_SECONDS / state.time_step)

    # Set initial mass mixing ratios
    bin_mmr = np.zeros((test_params.nbin, num_timesteps))
    for i_bin in range(test_params.nbin):
      bin_mmr[i_bin, 0] = 5.9 / (test_params.deltaz * 2.57474699e14) / state.air_density
      state.set_bin(i_bin+1, 1, [bin_mmr[i_bin, 0]])

    for i_time_step in range(num_timesteps):
        state.step()
        for i_bin in range(test_params.nbin):
            bin_mmr[i_bin, i_time_step] = state.get_bin(i_bin+1, 1)["total_mass_mixing_ratio"][0]

    # print the results
    print("Mass mixing ratios for Aluminum particles over time:")
    for i_bin in range(test_params.nbin):
        print(f"Bin {i_bin}: {bin_mmr[i_bin, :]}")



