import musica.mechanism_configuration as mc


def get_fully_defined_mechanism() -> mc.Mechanism:
    # Chemical species
    A = mc.Species(name="A", molecular_weight_kg_mol=0.02897, other_properties={"__absolute tolerance": "1.0e-30"})
    B = mc.Species(name="B", constant_concentration_mol_m3=1e19)
    C = mc.Species(name="C", constant_mixing_ratio_mol_mol=1e-20)
    M = mc.Species(name="M", is_third_body=True)
    H2O2 = mc.Species(
        name="H2O2",
        molecular_weight_kg_mol=0.0340147,
        other_properties={"__absolute tolerance": "1.0e-10"},
    )
    ethanol = mc.Species(
        name="ethanol",
        molecular_weight_kg_mol=0.04607,
        other_properties={"__absolute tolerance": "1.0e-20"},
    )

    # Chemical phases
    gas = mc.Phase(
        name="gas",
        species=[
            mc.PhaseSpecies(
                name=A.name,
                diffusion_coefficient_m2_s=1.0),
            B,
            C,
            ethanol,
            H2O2,
            M])

    # Reactions
    my_arrhenius = mc.Arrhenius(
        name="my arrhenius",
        A=32.1, B=-2.3, C=102.3, D=63.4, E=-1.3,
        gas_phase=gas,
        reactants=[B],
        products=[C],
        other_properties={"__irrelevant": "2"},
    )

    my_other_arrhenius = mc.Arrhenius(
        name="my other arrhenius",
        A=29.3, B=-1.5, Ea=101.2, D=82.6, E=-0.98,
        gas_phase=gas,
        reactants=[A],
        products=[(1.2, B)]
    )

    my_troe = mc.Troe(
        name="my troe",
        gas_phase=gas,
        k0_A=1.2e-12,
        k0_B=167,
        k0_C=3,
        kinf_A=136,
        kinf_B=5,
        kinf_C=24,
        Fc=0.9,
        N=0.8,
        reactants=[B, M],
        products=[C],
        other_properties={"__irrelevant": "2"},
    )

    my_ternary = mc.TernaryChemicalActivation(
        name="my ternary chemical activation",
        gas_phase=gas,
        k0_A=32.1,
        k0_B=-2.3,
        k0_C=102.3,
        kinf_A=63.4,
        kinf_B=-1.3,
        kinf_C=908.5,
        Fc=1.3,
        N=32.1,
        reactants=[B, M],
        products=[C],
        other_properties={"__irrelevant": "2"},
    )

    my_branched = mc.Branched(
        name="my branched",
        gas_phase=gas,
        reactants=[A],
        alkoxy_products=[B],
        nitrate_products=[C],
        X=1.2e-4,
        Y=167,
        a0=0.15,
        n=9,
        other_properties={"__irrelevant": "2"},
    )

    my_tunneling = mc.Tunneling(
        name="my tunneling",
        gas_phase=gas,
        reactants=[B],
        products=[C],
        A=123.45,
        B=1200.0,
        C=1.0e8,
        other_properties={"__irrelevant": "2"},
    )

    my_surface = mc.Surface(
        name="my surface",
        gas_phase=gas,
        gas_phase_species=A,
        reaction_probability=2.0e-2,
        gas_phase_products=[B, C],
        other_properties={"__irrelevant": "2"},
    )

    photo_B = mc.Photolysis(
        name="photo B",
        gas_phase=gas,
        reactants=[B],
        products=[C],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"},
    )

    my_emission = mc.Emission(
        name="my emission",
        gas_phase=gas,
        products=[B],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"},
    )

    my_first_order_loss = mc.FirstOrderLoss(
        name="my first order loss",
        gas_phase=gas,
        reactants=[C],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"},
    )

    user_defined = mc.UserDefined(
        name="my user defined",
        gas_phase=gas,
        reactants=[A, B],
        products=[(1.3, C)],
        scaling_factor=12.3,
        other_properties={"__irrelevant": "2"}
    )

    taylor_series_reaction = mc.TaylorSeries(
        name="my taylor series",
        gas_phase=gas,
        A=12.3,
        B=-1.5,
        C=1.0e-6,
        D=340,
        E=0.00032,
        reactants=[B],
        products=[C],
        other_properties={"__irrelevant": "2"}
    )

    # Mechanism
    return mc.Mechanism(
        name="Full Configuration",
        species=[A, B, C, M, H2O2, ethanol],
        phases=[gas],
        reactions=[my_arrhenius, my_other_arrhenius, my_troe, my_ternary, my_branched,
                   my_tunneling, my_surface, photo_B,
                   my_emission, my_first_order_loss, user_defined,
                   taylor_series_reaction
                   ],
        version=mc.Version(1, 0, 0),
    )


def _validate_species(species):
    # Define the expected species and their required attributes
    expected_species = {
        "A": {
            "molecular_weight_kg_mol": 0.02897,
            "other_properties": {"__absolute tolerance": "1e-30"}
        },
        "B": {
            "constant_concentration_mol_m3": 1e19,
        },
        "C": {
            "constant_mixing_ratio_mol_mol": 1e-20,
        },
        "M": {"is_third_body": True},
        "H2O2": {
            "molecular_weight_kg_mol": 0.0340147,
            "other_properties": {"__absolute tolerance": "1e-10"},
        },
        "ethanol": {
            "molecular_weight_kg_mol": 0.04607,
            "other_properties": {"__absolute tolerance": "1e-20"},
        }
    }

    # Create a dictionary for quick lookup of species by name
    species_dict = {sp.name: sp for sp in species}

    # Validate each expected species
    for name, attributes in expected_species.items():
        assert name in species_dict, f"Species '{name}' is missing."
        for attr, expected_value in attributes.items():
            assert hasattr(
                species_dict[name], attr
            ), f"Attribute '{attr}' is missing for species '{name}'."
            got_value = getattr(species_dict[name], attr)
            # Handle special cases for floating-point representation
            if isinstance(got_value, str) and ".0e" in got_value:
                got_value = got_value.replace(".0e", "e")
            elif isinstance(got_value, dict):
                def replace_in_dict(d):
                    for key, value in d.items():
                        if isinstance(value, str) and ".0e" in value:
                            d[key] = value.replace(".0e", "e")
                        elif isinstance(value, dict):
                            replace_in_dict(value)
                replace_in_dict(got_value)
            assert got_value == expected_value, (
                f"Attribute '{attr}' for species '{name}' has value "
                f"{got_value}, expected {expected_value}."
            )


def _validate_phases(phases):
    assert len(phases) == 1
    assert phases[0].name == "gas"
    assert phases[0].species[0].name == "A"
    assert phases[0].species[0].diffusion_coefficient_m2_s == 1.0
    assert phases[0].species[1].name == "B"
    assert phases[0].species[2].name == "C"
    assert phases[0].species[3].name == "ethanol"
    assert phases[0].species[4].name == "H2O2"


def _extract_components(components):
    return [
        {"species name": component.species_name, "coefficient": component.coefficient}
        for component in components
    ]


def _validate_arrhenius(reactions):
    assert reactions[0].type == mc.ReactionType.Arrhenius
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "B", "coefficient": 1}
    ]
    assert _extract_components(reactions[0].products) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].A == 32.1
    assert reactions[0].B == -2.3
    assert reactions[0].C == 102.3
    assert reactions[0].D == 63.4
    assert reactions[0].E == -1.3
    assert reactions[0].name == "my arrhenius"
    assert reactions[0].other_properties == {"__irrelevant": "2"}
    assert reactions[1].type == mc.ReactionType.Arrhenius
    assert _extract_components(reactions[1].reactants) == [
        {"species name": "A", "coefficient": 1}
    ]
    assert _extract_components(reactions[1].products) == [
        {"species name": "B", "coefficient": 1.2}
    ]
    assert reactions[1].A == 29.3
    assert reactions[1].B == -1.5
    assert reactions[1].C == -101.2 / 1.380649e-23
    assert reactions[1].D == 82.6
    assert reactions[1].E == -0.98
    assert reactions[1].name == "my other arrhenius"
    assert reactions[1].other_properties == {}


def _validate_emission(reactions):
    assert reactions[0].type == mc.ReactionType.Emission
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].products) == [
        {"species name": "B", "coefficient": 1}
    ]
    assert reactions[0].scaling_factor == 12.3
    assert reactions[0].name == "my emission"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_first_order_loss(reactions):
    assert reactions[0].type == mc.ReactionType.FirstOrderLoss
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].scaling_factor == 12.3
    assert reactions[0].name == "my first order loss"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_photolysis(reactions):
    assert reactions[0].type == mc.ReactionType.Photolysis
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "B", "coefficient": 1}
    ]
    assert _extract_components(reactions[0].products) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].scaling_factor == 12.3
    assert reactions[0].name == "photo B"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_surface(reactions):
    assert reactions[0].type == mc.ReactionType.Surface
    assert reactions[0].gas_phase == "gas"
    assert _extract_components([reactions[0].gas_phase_species]) == [
        {"species name": "A", "coefficient": 1}
    ]
    assert reactions[0].reaction_probability == 2.0e-2
    assert _extract_components(reactions[0].gas_phase_products) == [
        {"species name": "B", "coefficient": 1},
        {"species name": "C", "coefficient": 1},
    ]
    assert reactions[0].name == "my surface"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_troe(reactions):
    assert reactions[0].type == mc.ReactionType.Troe
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "B", "coefficient": 1},
        {"species name": "M", "coefficient": 1},
    ]
    assert _extract_components(reactions[0].products) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].k0_A == 1.2e-12
    assert reactions[0].k0_B == 167
    assert reactions[0].k0_C == 3
    assert reactions[0].kinf_A == 136
    assert reactions[0].kinf_B == 5
    assert reactions[0].kinf_C == 24
    assert reactions[0].Fc == 0.9
    assert reactions[0].N == 0.8
    assert reactions[0].name == "my troe"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_ternary_chemical_activation(reactions):
    assert reactions[0].type == mc.ReactionType.TernaryChemicalActivation
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "B", "coefficient": 1},
        {"species name": "M", "coefficient": 1},
    ]
    assert _extract_components(reactions[0].products) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].k0_A == 32.1
    assert reactions[0].k0_B == -2.3
    assert reactions[0].k0_C == 102.3
    assert reactions[0].kinf_A == 63.4
    assert reactions[0].kinf_B == -1.3
    assert reactions[0].kinf_C == 908.5
    assert reactions[0].Fc == 1.3
    assert reactions[0].N == 32.1
    assert reactions[0].name == "my ternary chemical activation"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_branched_no_ro2(reactions):
    assert reactions[0].type == mc.ReactionType.Branched
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "A", "coefficient": 1}
    ]
    assert _extract_components(reactions[0].alkoxy_products) == [
        {"species name": "B", "coefficient": 1}
    ]
    assert _extract_components(reactions[0].nitrate_products) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].X == 1.2e-4
    assert reactions[0].Y == 167
    assert reactions[0].a0 == 0.15
    assert reactions[0].n == 9
    assert reactions[0].name == "my branched"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_tunneling(reactions):
    assert reactions[0].type == mc.ReactionType.Tunneling
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "B", "coefficient": 1}
    ]
    assert _extract_components(reactions[0].products) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].A == 123.45
    assert reactions[0].B == 1200.0
    assert reactions[0].C == 1.0e8
    assert reactions[0].name == "my tunneling"
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_user_defined(reactions):
    assert reactions[0].type == mc.ReactionType.UserDefined
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "A", "coefficient": 1},
        {"species name": "B", "coefficient": 1},
    ]
    assert _extract_components(reactions[0].products) == [
        {"species name": "C", "coefficient": 1.3}
    ]
    assert reactions[0].scaling_factor == 12.3
    assert reactions[0].name == "my user defined"
    # assert reactions[0].other_properties == {}
    assert reactions[0].other_properties == {"__irrelevant": "2"}


def _validate_taylor_series(reactions):
    assert reactions[0].type == mc.ReactionType.TaylorSeries
    assert reactions[0].gas_phase == "gas"
    assert _extract_components(reactions[0].reactants) == [
        {"species name": "B", "coefficient": 1}
    ]
    assert _extract_components(reactions[0].products) == [
        {"species name": "C", "coefficient": 1}
    ]
    assert reactions[0].A == 12.3
    assert reactions[0].B == -1.5
    assert reactions[0].C == 1.0e-6
    assert reactions[0].D == 340
    assert reactions[0].E == 0.00032
    assert reactions[0].name == "my taylor series"


def validate_full_v1_mechanism(mechanism):
    assert mechanism is not None
    assert mechanism.name == "Full Configuration"
    assert len(mechanism.species) == 6
    _validate_species(mechanism.species)
    assert len(mechanism.phases) == 1
    _validate_phases(mechanism.phases)
    assert len(mechanism.reactions.arrhenius) == 2
    _validate_arrhenius(mechanism.reactions.arrhenius)
    assert len(mechanism.reactions.branched) == 1
    _validate_branched_no_ro2(mechanism.reactions.branched)
    assert len(mechanism.reactions.emission) == 1
    _validate_emission(mechanism.reactions.emission)
    assert len(mechanism.reactions.first_order_loss) == 1
    _validate_first_order_loss(mechanism.reactions.first_order_loss)
    assert len(mechanism.reactions.photolysis) == 1
    _validate_photolysis(mechanism.reactions.photolysis)
    assert len(mechanism.reactions.surface) == 1
    _validate_surface(mechanism.reactions.surface)
    assert len(mechanism.reactions.troe) == 1
    _validate_troe(mechanism.reactions.troe)
    assert len(mechanism.reactions.ternary_chemical_activation) == 1
    _validate_ternary_chemical_activation(mechanism.reactions.ternary_chemical_activation)
    assert len(mechanism.reactions.tunneling) == 1
    _validate_tunneling(mechanism.reactions.tunneling)
    assert len(mechanism.reactions.user_defined) == 1
    _validate_user_defined(mechanism.reactions.user_defined)
    assert len(mechanism.reactions.taylor_series) == 1
    _validate_taylor_series(mechanism.reactions.taylor_series)
    assert mechanism.version.major == 1
    assert mechanism.version.minor == 0
    assert mechanism.version.patch == 0
    assert len(mechanism.reactions) == 12
    for reaction in mechanism.reactions:
        assert reaction is not None
        assert isinstance(reaction.type, mc.ReactionType)
