import pytest
import os
import musica.mechanism_configuration as mc
from musica.mechanism_configuration import Mechanism, parse
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


def _get_aerosol_mechanism():
    """A mechanism with a full aerosol section that satisfies the aerosol model validator."""
    A = mc.Species(name="A", molecular_weight_kg_mol=0.05)
    B = mc.Species(name="B")
    H2O = mc.Species(name="H2O", molecular_weight_kg_mol=0.018)

    gas = mc.Phase(name="gas", species=[mc.PhaseSpecies(name="A", diffusion_coefficient_m2_s=1.5e-5)])
    aqueous = mc.Phase(name="aqueous", species=[
        mc.PhaseSpecies(name="A"),
        mc.PhaseSpecies(name="B"),
        mc.PhaseSpecies(name="H2O", density_kg_m3=1000.0),
    ])

    aerosol = mc.Aerosol(
        representations=[
            mc.SingleMomentMode(name="aitken", phases=[aqueous],
                                geometric_mean_radius=1e-6, geometric_standard_deviation=1e-5),
            mc.UniformSection(name="coarse", phases=[aqueous], min_radius=1e-6, max_radius=1e-5),
            mc.TwoMomentMode(name="fine", phases=[aqueous], geometric_standard_deviation=1.6),
        ],
        processes=[
            mc.HenrysLawPhaseTransfer(
                gas_phase="gas", gas_species="A", condensed_phase="aqueous",
                condensed_species="A", solvent="H2O",
                henrys_law_constant=mc.HenrysLawConstant(HLC_ref=1e-2, C=3000.0),
                accommodation_coefficient=0.1),
            mc.DissolvedReaction(
                phase="aqueous", solvent="H2O",
                reactants=[mc.Species(name="A")], products=[mc.Species(name="B")],
                rate_constant=mc.Arrhenius(A=1e3, C=100.0)),
            mc.DissolvedReversibleReaction(
                phase="aqueous", solvent="H2O",
                reactants=[mc.Species(name="A")], products=[mc.Species(name="B")],
                forward_rate_constant=mc.Arrhenius(A=1e3, C=100.0),
                equilibrium_constant=mc.Equilibrium(A=1725.0, C=0.0)),
        ],
        constraints=[
            mc.HenrysLawEquilibrium(
                gas_phase="gas", gas_species="A", condensed_phase="aqueous",
                condensed_species="A", solvent="H2O",
                henrys_law_constant=mc.HenrysLawConstant(HLC_ref=1e-2, C=3000.0)),
            mc.DissolvedEquilibrium(
                phase="aqueous", solvent="H2O",
                reactants=[mc.Species(name="A")], products=[mc.Species(name="B")],
                algebraic_species="A", equilibrium_constant=mc.Equilibrium(A=1.0)),
            mc.LinearConstraint(
                algebraic_phase="gas", algebraic_species="A",
                terms=[mc.LinearConstraintTerm("gas", "A", 1.0)],
                constant=mc.DiagnoseFromState()),
            mc.LinearConstraint(
                algebraic_phase="aqueous", algebraic_species="B",
                terms=[mc.LinearConstraintTerm("aqueous", "B", 1.0)],
                constant=mc.FixedConstant(3e-8)),
        ],
    )
    return mc.Mechanism(name="aerosol test", version=mc.Version(1, 0, 0),
                        species=[A, B, H2O], phases=[gas, aqueous], aerosol=aerosol)


def test_aerosol_serialize_export_loop(tmp_path):
    mechanism = _get_aerosol_mechanism()
    for extension in (".json", ".yml", ".yaml"):
        path = f"{tmp_path}/aerosol_mechanism{extension}"
        mechanism.export(path)
        parsed = parse(path)
        assert parsed.aerosol is not None
        assert len(parsed.aerosol.representations) == 3
        assert len(parsed.aerosol.processes) == 3
        assert len(parsed.aerosol.constraints) == 4


def test_mechanism_export_loop(tmp_path):
    mechanism = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"{tmp_path}/test_mechanism{extension}"
        mechanism.export(path)
        mechanism = parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_parser_loop(tmp_path):
    mechanism = get_fully_defined_mechanism()
    extensions = [".json", ".yml", ".yaml"]
    for extension in extensions:
        path = f"{tmp_path}/test_mechanism{extension}"
        mechanism.export(path)
        mechanism = parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_to_file(tmp_path):
    mechanism = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        file_path = f'{tmp_path}/test_mechanism{extension}'
        assert not os.path.exists(file_path)
        mechanism.export(file_path)
        assert os.path.exists(file_path)


def test_bad_inputs():
    mechanism = Mechanism(name="Full Configuration")
    with pytest.raises(Exception):
        mechanism.export("unsupported.txt")


def test_path_creation(tmp_path):
    mechanism = Mechanism(name="Full Configuration")
    path = f"{tmp_path}/non_existant_path/"
    assert not os.path.exists(path)
    mechanism.export(f"{path}test_mechanism.json")
    assert os.path.exists(path)


def test_overwrite_file(tmp_path):
    mechanism = Mechanism(name="Full Configuration")
    file_path = f'{tmp_path}/test_mechanism.json'
    assert not os.path.exists(file_path)

    # write first file
    mechanism.export(file_path)
    files = list(tmp_path.iterdir())
    assert len(files) == 1

    # overwrite file
    mechanism.export(file_path)
    files = list(tmp_path.iterdir())
    assert len(files) == 1


if __name__ == "__main__":
    pytest.main([__file__])
