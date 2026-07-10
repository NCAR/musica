"""CAM aerosol-distribution configurations for miam

Construct miam objects that reproduce the aerosol size distrutions for
- BAM (bulk)
- MAM (modal)
- CARMA (sectional)

Each of these representations are then connected to the same cloud-sulfate aqueous chemistry
from the initial cloud integration test.

Each representation was broken down in issue https://github.com/NCAR/musica/issues/881, check 
there for where the numbers come from.
"""

import musica.mechanism_configuration as mc
from musica.miam import (
    ArrheniusRateConstant,
    EquilibriumConstant,
    HenryLawConstant,
    UniformSection,
    SingleMomentMode,
    TwoMomentMode,
    DissolvedReaction,
    DissolvedReversibleReaction,
    HenryLawEquilibriumConstraint,
    DissolvedEquilibriumConstraint,
    LinearConstraint,
    LinearConstraintTerm,
    Model,
)

# Unit-conversion constants
M_ATM_TO_MOL_M3_PA = 1000.0 / 101325.0
C_H2O_M = 55.556           # mol/L — pure-water molarity (rate/Keq unit conversion only)
MW_H2O = 0.018             # kg/mol
RHO_H2O = 1000.0           # kg/m3
UM = 1.0e-6                # micrometre -> m
CM_TO_M = 1.0e-2           # cm -> m


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
    for varient, modes in MAM_MODES.items():
        reps[varient] = [
            TwoMomentMode(
                name=name.upper(),
                phase_names=[f"{name.upper()}_AQ"],
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
        ("SO4",     6.95e-6 * CM_TO_M, 2.03),
        ("OC",      2.12e-6 * CM_TO_M, 2.20),
        ("BC",      1.18e-6 * CM_TO_M, 2.00),
        ("NH4NO3",  6.95e-6 * CM_TO_M, 2.03),
    ]
    # BAM dust/sea-salt are binned; edges/diameters in meteres here
    BAM_DUST_EDGES_UM = [0.1, 1.0, 2.5, 5.0, 10.0]   # diameter bin edges
    BAM_DUST_INTRABIN_GEOMETRIC_STANDARD_DEVIATION = 2.0
    BAM_SEASALT_DIAMETER_M = [0.52 * UM, 2.38 * UM, 4.86 * UM, 15.14 * UM]  # dry mass-weighted diameters
    BAM_SEASALT_GEOMETRIC_STANDARD_DEVIATION = 2.0   # placeholder: BAM sea salt has no explicit dry standard deviation (Gerber growth)

    reps = []
    for name, geometric_mean_radius, sigma_g in BAM_BULK:
        reps.append(SingleMomentMode(
            name=name, phase_names=[f"{name}_AQ"],
            geometric_mean_radius=geometric_mean_radius, geometric_standard_deviation=sigma_g,
        ))
    # dust bins: geometric-mean radius of each diameter-edge pair, / 2 for radius
    for i in range(len(BAM_DUST_EDGES_UM) - 1):
        diameter_lower_edge, diameter_upper_edge = BAM_DUST_EDGES_UM[i], BAM_DUST_EDGES_UM[i + 1]
        geometric_mean_radius = 0.5 * (diameter_lower_edge * diameter_upper_edge) ** 0.5 * UM
        nm = f"DST{i + 1:02d}"
        reps.append(SingleMomentMode(
            name=nm, phase_names=[f"{nm}_AQ"],
            geometric_mean_radius=geometric_mean_radius, geometric_standard_deviation=BAM_DUST_INTRABIN_GEOMETRIC_STANDARD_DEVIATION,
        ))
    # sea-salt bins: prescribed dry mass-weighted diameter -> radius
    for i, diameter_m in enumerate(BAM_SEASALT_DIAMETER_M):
        nm = f"SSLT{i + 1:02d}"
        reps.append(SingleMomentMode(
            name=nm, phase_names=[f"{nm}_AQ"],
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
        representations.append(UniformSection(
            name=section_name, phase_names=[f"{section_name}_AQ"],
            min_radius=center_radius / edge_factor, max_radius=center_radius * edge_factor,
        ))
    return representations


def carma_representations(case):
    """ Create a CARMA representation using UniformSection per bin for the given case.
    """
    # CARMA cases: (group name, number of bins, minimum radius (m), volume ratio)
    CARMA_CASES = {
    "dust":          [("dust",         16, 1.19e-5 * CM_TO_M, 2.371)],
    "sea_salt":      [("sea_salt",     16, 1.0e-6  * CM_TO_M, 4.32)],
    "sulfate":       [("sulfate",      30, 3.43230298e-8 * CM_TO_M, 2.4)],
    "meteor_smoke":  [("meteor_smoke", 28, 2.0e-8  * CM_TO_M, 2.0)],
    "mixed_sulfate": [("meteor_smoke", 28, 2.0e-8  * CM_TO_M, 2.0),
                    ("sulfate",      28, 3.43230298e-8 * CM_TO_M, 2.56)],
    }
    representations = []
    for group_name, number_of_bins, minimum_radius, volume_ratio in CARMA_CASES[case]:
        representations.extend(carma_sections(group_name, number_of_bins, minimum_radius, volume_ratio))
    return representations


def gas_species_and_phase():
    """Shared gas phase: SO2, H2O2, O3."""
    so2 = mc.Species(name="SO2")
    h2o2 = mc.Species(name="H2O2")
    o3 = mc.Species(name="O3")
    gas = mc.Phase(name="gas", species=[so2, h2o2, o3])
    return [so2, h2o2, o3], gas


def sulfate_chemistry(repr_name, phase_name):
    """Cloud-sulfate S(IV)->S(VI) aqueous chemistry local to one representation.

    Mirrors _create_kinetics_model() from the integration test: Henry's-law
    uptake of SO2/H2O2/O3, the S(IV) acid/dissociation equilibria, and the
    H2O2 + O3 oxidation pathways producing SO4--.  All processes/constraints
    reference this representation's own aqueous phase, so the same chemistry
    can run independently on every mode/bin.

    Returns (aqueous_species, aqueous_phase, processes, constraints).  The
    IC-dependent mass-budget LinearConstraints from the test are intentionally
    omitted — those belong to the box-model driver, not the reusable config.
    """
    # aqueous species (one fresh set per representation)
    names = ["SO2_aq", "H2O2_aq", "O3_aq", "Hp", "OHm",
             "HSO3m", "SO3mm", "SO4mm", "SO2OOHm", "H2O"]
    sp = {n: mc.Species(name=n) for n in names}
    sp["H2O"].molecular_weight_kg_mol = MW_H2O
    sp["H2O"].density_kg_m3 = RHO_H2O
    aq_species = list(sp.values())
    aq_phase = mc.Phase(name=phase_name, species=aq_species)

    # ── kinetic reactions ──
    processes = [
        DissolvedReversibleReaction(          # R1a: HSO3- + H2O2 <=> SO2OOH- + H2O
            phase_name=phase_name,
            reactant_names=["HSO3m", "H2O2_aq"], product_names=["SO2OOHm", "H2O"],
            solvent_name="H2O",
            forward_rate_constant=ArrheniusRateConstant(A=C_H2O_M * (7.45e7 / 13.0), C=4430.0),
            equilibrium_constant=EquilibriumConstant(A=1725.0),
        ),
        DissolvedReaction(                    # R1b: SO2OOH- + H+ -> SO4--
            representation_name=repr_name, phase_name=phase_name,
            reactant_names=["SO2OOHm", "Hp"], product_names=["SO4mm"], solvent_name="H2O",
            rate_constant=ArrheniusRateConstant(A=C_H2O_M * 2.4e6, C=4430.0),
        ),
        DissolvedReaction(                    # R2: HSO3- + O3 -> SO4-- + H+
            representation_name=repr_name, phase_name=phase_name,
            reactant_names=["HSO3m", "O3_aq"], product_names=["SO4mm", "Hp"], solvent_name="H2O",
            rate_constant=ArrheniusRateConstant(A=C_H2O_M * 3.75e5, C=5530.0),
        ),
        DissolvedReaction(                    # R3: SO3-- + O3 -> SO4--
            representation_name=repr_name, phase_name=phase_name,
            reactant_names=["SO3mm", "O3_aq"], product_names=["SO4mm"], solvent_name="H2O",
            rate_constant=ArrheniusRateConstant(A=C_H2O_M * 1.59e9, C=5280.0),
        ),
    ]

    # ── constraints (all local to this aqueous phase) ──
    constraints = []
    for gas_name, aq_name, hlc_ref, c in [
        ("SO2", "SO2_aq", 1.23, 3120.0),
        ("H2O2", "H2O2_aq", 7.4e4, 6621.0),
        ("O3", "O3_aq", 1.15e-2, 2560.0),
    ]:
        constraints.append(HenryLawEquilibriumConstraint(
            gas_species_name=gas_name, condensed_species_name=aq_name,
            solvent_name="H2O", condensed_phase_name=phase_name,
            henry_law_constant=HenryLawConstant(HLC_ref=hlc_ref * M_ATM_TO_MOL_M3_PA, C=c),
            solvent_molecular_weight=MW_H2O, solvent_density=RHO_H2O,
        ))
    constraints += [
        DissolvedEquilibriumConstraint(       # Kw
            phase_name=phase_name, reactant_names=["H2O"], product_names=["Hp", "OHm"],
            algebraic_species_name="OHm", solvent_name="H2O",
            equilibrium_constant=EquilibriumConstant(A=1e-14 / (C_H2O_M * C_H2O_M), C=0.0),
        ),
        DissolvedEquilibriumConstraint(       # Ka1
            phase_name=phase_name, reactant_names=["SO2_aq"], product_names=["HSO3m", "Hp"],
            algebraic_species_name="HSO3m", solvent_name="H2O",
            equilibrium_constant=EquilibriumConstant(A=1.7e-2 / C_H2O_M, C=2090.0),
        ),
        DissolvedEquilibriumConstraint(       # Ka2
            phase_name=phase_name, reactant_names=["HSO3m"], product_names=["SO3mm", "Hp"],
            algebraic_species_name="SO3mm", solvent_name="H2O",
            equilibrium_constant=EquilibriumConstant(A=6.0e-8 / C_H2O_M, C=1120.0),
        ),
        LinearConstraint(                     # charge balance
            algebraic_phase_name=phase_name, algebraic_species_name="Hp",
            terms=[
                LinearConstraintTerm(phase_name, "Hp", 1.0),
                LinearConstraintTerm(phase_name, "OHm", -1.0),
                LinearConstraintTerm(phase_name, "HSO3m", -1.0),
                LinearConstraintTerm(phase_name, "SO3mm", -2.0),
                LinearConstraintTerm(phase_name, "SO4mm", -2.0),
                LinearConstraintTerm(phase_name, "SO2OOHm", -1.0),
            ],
            constant=0.0,
        ),
    ]
    return aq_species, aq_phase, processes, constraints


def build_model(name, representations):
    """Assemble a miam Model: gas phase + cloud-sulfate chemistry on each
    representation's own aqueous phase."""
    gas_species, gas_phase = gas_species_and_phase()
    species = list(gas_species)
    condensed_phases = []
    processes = []
    constraints = []
    for rep in representations:
        phase_name = rep.phase_names[0]
        aq_species, aq_phase, procs, cons = sulfate_chemistry(rep.name, phase_name)
        species += aq_species
        condensed_phases.append(aq_phase)
        processes += procs
        constraints += cons
    return Model(
        name=name, species=species, condensed_phases=condensed_phases,
        representations=representations, processes=processes, constraints=constraints,
    )


def all_configs():
    """One Model per entry in the doc's tables."""
    configs = {}
    # mam_representations() returns {variant: [modes]} for all MAM variants
    for variant, representations in mam_representations().items():
        configs[variant] = build_model(variant, representations)
    configs["bam"] = build_model("bam", bam_representations())
    for case in ("dust", "sea_salt", "sulfate", "meteor_smoke", "mixed_sulfate"):
        configs[f"carma_{case}"] = build_model(f"carma_{case}", carma_representations(case))
    return configs


if __name__ == "__main__":
    for cfg_name, model in all_configs().items():
        print(f"{cfg_name:20s}  representations={len(model.representations):2d}  "
              f"species={len(model.species):3d}  processes={len(model.processes):3d}  "
              f"constraints={len(model.constraints):3d}")
