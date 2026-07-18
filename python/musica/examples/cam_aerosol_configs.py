"""CAM aerosol-distribution configurations for miam

Construct musica mechanisms with an aerosol section that reproduce the aerosol
size distrutions for
- BAM (bulk)
- MAM (modal)
- CARMA (sectional)

Each of these representations are then connected to the same cloud-sulfate aqueous chemistry
from the initial cloud integration test.

Each representation was broken down in issue https://github.com/NCAR/musica/issues/881, check
there for where the numbers come from.
"""

import musica.mechanism_configuration as mc

# Unit-conversion constants
M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556           # mol/L — pure-water molarity (rate/Keq unit conversion only)
MW_H2O = 0.018             # kg/mol
RHO_H2O = 1000.0           # kg/m3
UM = 1.0e-6                # micrometre -> m
CM_TO_M = 1.0e-2           # cm -> m

# ─── Defaults for kinetic (mass-transfer-limited) Henry's-law uptake ─────────
# Kinetic uptake needs particle effective radius, number, and phase volume
# fraction — all derived by miam from condensed-phase VOLUME = Σ [species]·MW/ρ.
# So every species needs a molecular weight and density.  These are sensible,
# illustrative defaults (solutes given water-like density); not evaluated values.
AQUEOUS_PROPERTIES = {  # species: (molecular weight [kg/mol], density [kg/m3])
    "H2O": (MW_H2O, RHO_H2O),
    "SO2_aq": (0.064, 1000.0),
    "H2O2_aq": (0.034, 1000.0),
    "O3_aq": (0.048, 1000.0),
    "Hp": (0.001, 1000.0),
    "OHm": (0.017, 1000.0),
    "HSO3m": (0.081, 1000.0),
    "SO3mm": (0.080, 1000.0),
    "SO4mm": (0.096, 1000.0),
    "SO2OOHm": (0.097, 1000.0),
}
# Gas molecular weights [kg/mol] (mean thermal speed in the uptake rate).
GAS_MOLECULAR_WEIGHT = {"SO2": 0.064, "H2O2": 0.034, "O3": 0.048}
# Gas diffusion coefficient [m2/s] and mass accommodation coefficient [-].
# Approximate literature values (SO2 from the miam unit test).
GAS_UPTAKE = {  # gas: (diffusion_coefficient, accommodation_coefficient)
    "SO2": (1.28e-5, 0.11),
    "H2O2": (1.46e-5, 0.11),
    "O3": (1.48e-5, 2.0e-3),
}


def mam_representations():
    """ Create a dictionary of TwoMomentMode representations for all MAM variants
    """
    MAM_MODES = {
        "mam3": [
            ("accum", 1.8), ("aitken", 1.6), ("coarse", 1.8),
        ],
        "mam4": [
            ("accum", 1.8), ("aitken", 1.6), ("coarse", 1.8), ("primary_carbon", 1.6),
        ],
        "mam5": [
            ("accum", 1.8), ("aitken", 1.6), ("coarse", 1.8), ("primary_carbon", 1.6),
            ("coarse_strat", 1.2),
        ],
        "mam7": [
            ("accum", 1.8), ("aitken", 1.6), ("primary_carbon", 1.6),
            ("fine_seasalt", 2.0), ("fine_dust", 1.8),
            ("coarse_seasalt", 2.0), ("coarse_dust", 1.8),
        ],
    }

    """MAM modes -> one TwoMomentMode per mode (fixed sigma_g, prognostic n+mass)."""
    reps = {}
    for variant, modes in MAM_MODES.items():
        reps[variant] = [
            mc.TwoMomentMode(
                name=name.upper(),
                geometric_standard_deviation=sigma_g,
            ) for name, sigma_g in modes
        ]
    return reps


def bam_representations():
    """Create a BAM representation using a SingleMomentMode per bulk species and per dust/sea-salt bin.

    Bulk species use their prescribed (fixed) lognormal.  Binned species use a
    per-bin SingleMomentMode whose geometric mean radius is the bin's
    geometric-mean radius and whose sigma is the intra-bin value.
    """

    # species, geometric-mean radius (given in centimeters, converted to meters), geometric standard deviation
    BAM_BULK = [
        ("SO4", 6.95e-6 * CM_TO_M, 2.03),
        ("OC", 2.12e-6 * CM_TO_M, 2.20),
        ("BC", 1.18e-6 * CM_TO_M, 2.00),
        ("NH4NO3", 6.95e-6 * CM_TO_M, 2.03),
    ]
    # BAM dust/sea-salt are binned; edges/diameters in meteres here
    BAM_DUST_EDGES_UM = [0.1, 1.0, 2.5, 5.0, 10.0]   # diameter bin edges
    BAM_DUST_INTRABIN_GEOMETRIC_STANDARD_DEVIATION = 2.0
    BAM_SEASALT_DIAMETER_M = [0.52 * UM, 2.38 * UM, 4.86 * UM, 15.14 * UM]  # dry mass-weighted diameters
    # placeholder: BAM sea salt has no explicit dry standard deviation (Gerber growth)
    BAM_SEASALT_GEOMETRIC_STANDARD_DEVIATION = 2.0

    reps = []
    for name, geometric_mean_radius, sigma_g in BAM_BULK:
        reps.append(mc.SingleMomentMode(
            name=name,
            geometric_mean_radius=geometric_mean_radius, geometric_standard_deviation=sigma_g,
        ))
    # dust bins: geometric-mean radius of each diameter-edge pair, / 2 for radius
    for i in range(len(BAM_DUST_EDGES_UM) - 1):
        diameter_lower_edge, diameter_upper_edge = BAM_DUST_EDGES_UM[i], BAM_DUST_EDGES_UM[i + 1]
        geometric_mean_radius = 0.5 * (diameter_lower_edge * diameter_upper_edge) ** 0.5 * UM
        nm = f"DST{i + 1:02d}"
        reps.append(
            mc.SingleMomentMode(
                name=nm,
                geometric_mean_radius=geometric_mean_radius,
                geometric_standard_deviation=BAM_DUST_INTRABIN_GEOMETRIC_STANDARD_DEVIATION,
            ))
    # sea-salt bins: prescribed dry mass-weighted diameter -> radius
    for i, diameter_m in enumerate(BAM_SEASALT_DIAMETER_M):
        nm = f"SSLT{i + 1:02d}"
        reps.append(mc.SingleMomentMode(
            name=nm,
            geometric_mean_radius=0.5 * diameter_m,
            geometric_standard_deviation=BAM_SEASALT_GEOMETRIC_STANDARD_DEVIATION,
        ))
    return reps


def carma_sections(group_name, number_of_bins, minimum_radius, volume_ratio):
    """Make a CARMA group, which is represented as N UniformSections

    CARMA bin center radius:
        center_radius(i) = minimum_radius * volume_ratio**((i-1)/3),  i = 1..number_of_bins.
    Radius spans a factor volume_ratio**(1/3) per bin, so the geometric edges are
    center_radius / volume_ratio**(1/6)  and  center_radius * volume_ratio**(1/6).
    """
    representations = []
    edge_factor = volume_ratio ** (1.0 / 6.0)   # sqrt of the bin-to-bin radius ratio
    for bin_index in range(1, number_of_bins + 1):
        center_radius = minimum_radius * volume_ratio ** ((bin_index - 1) / 3.0)
        section_name = f"{group_name.upper()}_BIN{bin_index:02d}"
        representations.append(mc.UniformSection(
            name=section_name,
            min_radius=center_radius / edge_factor, max_radius=center_radius * edge_factor,
        ))
    return representations


def carma_representations(case):
    """ Create a CARMA representation using UniformSection per bin for the given case.
    """
    # CARMA cases: (group name, number of bins, minimum radius (m), volume ratio)
    CARMA_CASES = {
        "dust": [("dust", 16, 1.19e-5 * CM_TO_M, 2.371)],
        "sea_salt": [("sea_salt", 16, 1.0e-6 * CM_TO_M, 4.32)],
        "sulfate": [("sulfate", 30, 3.43230298e-8 * CM_TO_M, 2.4)],
        "meteor_smoke": [("meteor_smoke", 28, 2.0e-8 * CM_TO_M, 2.0)],
        "mixed_sulfate": [("meteor_smoke", 28, 2.0e-8 * CM_TO_M, 2.0),
                          ("sulfate", 28, 3.43230298e-8 * CM_TO_M, 2.56)],
    }
    representations = []
    for group_name, number_of_bins, minimum_radius, volume_ratio in CARMA_CASES[case]:
        representations.extend(carma_sections(group_name, number_of_bins, minimum_radius, volume_ratio))
    return representations


def gas_species_and_phase(kinetic_uptake=False):
    """Shared gas phase: SO2, H2O2, O3 (with molecular weights for uptake).

    When ``kinetic_uptake`` is True, each species also carries its gas-phase
    diffusion coefficient on the phase (required by HenryLawPhaseTransfer).
    """
    gas_species = []
    phase_species = []
    for name in ("SO2", "H2O2", "O3"):
        s = mc.Species(name=name)
        s.molecular_weight_kg_mol = GAS_MOLECULAR_WEIGHT[name]
        gas_species.append(s)
        if kinetic_uptake:
            diffusion_coefficient, _ = GAS_UPTAKE[name]
            phase_species.append(mc.PhaseSpecies(name=name, diffusion_coefficient_m2_s=diffusion_coefficient))
        else:
            phase_species.append(s)
    gas = mc.Phase(name="gas", species=phase_species)
    return gas_species, gas


def sulfate_chemistry(phase_name, gas_species_by_name, gas_phase, kinetic_uptake=False):
    """Cloud-sulfate S(IV)->S(VI) aqueous chemistry local to one representation.

    Mirrors _create_kinetics_mechanism() from the integration test: Henry's-law
    uptake of SO2/H2O2/O3, the S(IV) acid/dissociation equilibria, and the
    H2O2 + O3 oxidation pathways producing SO4--.  All processes/constraints
    reference this representation's own aqueous phase, so the same chemistry
    can run independently on every mode/bin.

    ``kinetic_uptake`` selects how the gases enter the condensed phase:
      - False (default): instantaneous ``HenryLawEquilibrium`` — this
        is size-INDEPENDENT (depends only on liquid-water volume), so different
        representations give identical chemistry.
      - True: mass-transfer-limited ``HenryLawPhaseTransfer`` whose rate scales
        with particle surface area (radius × number), so the size distribution
        drives the chemistry.  Requires species MW/ρ (set below) and — for
        TwoMomentMode — a NUMBER_CONCENTRATION initial condition from the driver.

    Returns (aqueous_species, aqueous_phase, processes, constraints).  The
    IC-dependent mass-budget LinearConstraints from the test are intentionally
    omitted — those belong to the box-model driver, not the reusable config.
    """
    # aqueous species (one fresh set per representation); MW/ρ needed so miam
    # can derive particle volume -> effective radius / number for kinetic uptake.
    sp = {n: mc.Species(name=n) for n in AQUEOUS_PROPERTIES}
    for n, (mw, rho) in AQUEOUS_PROPERTIES.items():
        sp[n].molecular_weight_kg_mol = mw
        sp[n].density_kg_m3 = rho
    aq_species = list(sp.values())
    aq_phase = mc.Phase(name=phase_name, species=aq_species)

    # ── kinetic reactions ──
    processes = [
        mc.DissolvedReversibleReaction(          # R1a: HSO3- + H2O2 <=> SO2OOH- + H2O
            phase=aq_phase, solvent=sp["H2O"],
            reactants=[sp["HSO3m"], sp["H2O2_aq"]], products=[sp["SO2OOHm"], sp["H2O"]],
            forward_rate_constant=mc.Equilibrium(A=C_H2O_M * (7.45e7 / 13.0), C=4430.0),
            equilibrium_constant=mc.Equilibrium(A=1725.0),
        ),
        mc.DissolvedReaction(                    # R1b: SO2OOH- + H+ -> SO4--
            phase=aq_phase, solvent=sp["H2O"],
            reactants=[sp["SO2OOHm"], sp["Hp"]], products=[sp["SO4mm"]],
            rate_constant=mc.Equilibrium(A=C_H2O_M * 2.4e6, C=4430.0),
        ),
        mc.DissolvedReaction(                    # R2: HSO3- + O3 -> SO4-- + H+
            phase=aq_phase, solvent=sp["H2O"],
            reactants=[sp["HSO3m"], sp["O3_aq"]], products=[sp["SO4mm"], sp["Hp"]],
            rate_constant=mc.Equilibrium(A=C_H2O_M * 3.75e5, C=5530.0),
        ),
        mc.DissolvedReaction(                    # R3: SO3-- + O3 -> SO4--
            phase=aq_phase, solvent=sp["H2O"],
            reactants=[sp["SO3mm"], sp["O3_aq"]], products=[sp["SO4mm"]],
            rate_constant=mc.Equilibrium(A=C_H2O_M * 1.59e9, C=5280.0),
        ),
    ]

    # ── gas -> aqueous transfer: equilibrium constraint OR kinetic uptake ──
    constraints = []
    for gas_name, aq_name, hlc_ref, c in [
        ("SO2", "SO2_aq", 1.23, 3120.0),
        ("H2O2", "H2O2_aq", 7.4e4, 6621.0),
        ("O3", "O3_aq", 1.15e-2, 2560.0),
    ]:
        hlc = mc.HenryLawConstant(HLC_ref=hlc_ref * M_ATM_TO_MOL_M3_PA, C=c)
        gas_species = gas_species_by_name[gas_name]
        if kinetic_uptake:
            diffusion_coefficient, accommodation_coefficient = GAS_UPTAKE[gas_name]
            processes.append(mc.HenryLawPhaseTransfer(   # rate ∝ surface area (radius × number)
                gas_phase=gas_phase, gas_species=gas_species,
                condensed_phase=aq_phase, condensed_species=sp[aq_name], solvent=sp["H2O"],
                henry_law_constant=hlc,
                diffusion_coefficient=diffusion_coefficient,
                accommodation_coefficient=accommodation_coefficient,
            ))
        else:
            constraints.append(mc.HenryLawEquilibrium(   # instantaneous, size-independent
                gas_phase=gas_phase, gas_species=gas_species,
                condensed_phase=aq_phase, condensed_species=sp[aq_name],
                solvent=sp["H2O"],
                henry_law_constant=hlc,
                solvent_molecular_weight=MW_H2O, solvent_density=RHO_H2O,
            ))
    constraints += [
        mc.DissolvedEquilibrium(       # Kw
            phase=aq_phase, reactants=[sp["H2O"]], products=[sp["Hp"], sp["OHm"]],
            algebraic_species=sp["OHm"], solvent=sp["H2O"],
            equilibrium_constant=mc.Equilibrium(A=1e-14 / (C_H2O_M * C_H2O_M), C=0.0),
        ),
        mc.DissolvedEquilibrium(       # Ka1
            phase=aq_phase, reactants=[sp["SO2_aq"]], products=[sp["HSO3m"], sp["Hp"]],
            algebraic_species=sp["HSO3m"], solvent=sp["H2O"],
            equilibrium_constant=mc.Equilibrium(A=1.7e-2 / C_H2O_M, C=2090.0),
        ),
        mc.DissolvedEquilibrium(       # Ka2
            phase=aq_phase, reactants=[sp["HSO3m"]], products=[sp["SO3mm"], sp["Hp"]],
            algebraic_species=sp["SO3mm"], solvent=sp["H2O"],
            equilibrium_constant=mc.Equilibrium(A=6.0e-8 / C_H2O_M, C=1120.0),
        ),
        mc.LinearConstraint(                     # charge balance
            algebraic_phase=aq_phase, algebraic_species=sp["Hp"],
            terms=[
                mc.LinearConstraintTerm(aq_phase, sp["Hp"], 1.0),
                mc.LinearConstraintTerm(aq_phase, sp["OHm"], -1.0),
                mc.LinearConstraintTerm(aq_phase, sp["HSO3m"], -1.0),
                mc.LinearConstraintTerm(aq_phase, sp["SO3mm"], -2.0),
                mc.LinearConstraintTerm(aq_phase, sp["SO4mm"], -2.0),
                mc.LinearConstraintTerm(aq_phase, sp["SO2OOHm"], -1.0),
            ],
            constant=mc.FixedConstant(0.0),
        ),
    ]
    return aq_species, aq_phase, processes, constraints


def build_model(name, representations, kinetic_uptake=False):
    """Assemble a Mechanism: gas phase + cloud-sulfate chemistry on each
    representation's own aqueous phase, collected into an aerosol section.

    Mutates each representation in ``representations`` to attach its own
    freshly-built aqueous phase (named ``f"{rep.name}_AQ"``).

    kinetic_uptake=True uses size-dependent HenryLawPhaseTransfer for gas uptake
    (see sulfate_chemistry); default False uses equilibrium partitioning.
    """
    gas_species, gas_phase = gas_species_and_phase(kinetic_uptake)
    gas_species_by_name = {s.name: s for s in gas_species}
    species = list(gas_species)
    phases = [gas_phase]
    processes = []
    constraints = []
    for rep in representations:
        phase_name = f"{rep.name}_AQ"
        aq_species, aq_phase, procs, cons = sulfate_chemistry(
            phase_name, gas_species_by_name, gas_phase, kinetic_uptake)
        rep.phases = [aq_phase]
        species += aq_species
        phases.append(aq_phase)
        processes += procs
        constraints += cons
    return mc.Mechanism(
        name=name, species=species, phases=phases, reactions=[],
        aerosol=mc.Aerosol(representations=representations, processes=processes, constraints=constraints),
    )


def all_configs():
    """One Mechanism per entry in the doc's tables."""
    configs = {}
    # mam_representations() returns {variant: [modes]} for all MAM variants
    for variant, representations in mam_representations().items():
        configs[variant] = build_model(variant, representations)
    configs["bam"] = build_model("bam", bam_representations())
    for case in ("dust", "sea_salt", "sulfate", "meteor_smoke", "mixed_sulfate"):
        configs[f"carma_{case}"] = build_model(f"carma_{case}", carma_representations(case))
    return configs


if __name__ == "__main__":
    for cfg_name, mechanism in all_configs().items():
        aerosol = mechanism.aerosol
        print(f"{cfg_name:20s}  representations={len(aerosol.representations):2d}  "
              f"species={len(mechanism.species):3d}  processes={len(aerosol.processes):3d}  "
              f"constraints={len(aerosol.constraints):3d}")
