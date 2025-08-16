import pytest
import musica.mechanism_configuration as mc
from musica.mechanism_configuration import Parser
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


def test_parsed_full_v1_configuration():
    parser = Parser()
    extensions = [".yaml", ".json"]
    for extension in extensions:
        path = f"musica/test/examples/v1/full_configuration/full_configuration{extension}"
        mechanism = parser.parse(path)
        validate_full_v1_mechanism(mechanism)


def test_parser_reports_bad_files():
    parser = Parser()
    extensions = [".yaml", ".json"]
    for extension in extensions:
        path = f"musica/test/examples/_missing_configuration{extension}"
        with pytest.raises(Exception):
            parser.parse(path)


def test_hard_coded_full_v1_configuration():
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    validate_full_v1_mechanism(MECHANISM_FULLY_DEFINED)


def test_hard_coded_default_constructed_types():
    arrhenius = mc.Arrhenius()
    assert arrhenius.type == mc.ReactionType.Arrhenius
    condensed_phase_arrhenius = mc.CondensedPhaseArrhenius()
    assert condensed_phase_arrhenius.type == mc.ReactionType.CondensedPhaseArrhenius
    condensed_phase_photolysis = mc.CondensedPhasePhotolysis()
    assert condensed_phase_photolysis.type == mc.ReactionType.CondensedPhasePhotolysis
    emission = mc.Emission()
    assert emission.type == mc.ReactionType.Emission
    first_order_loss = mc.FirstOrderLoss()
    assert first_order_loss.type == mc.ReactionType.FirstOrderLoss
    henrys_law = mc.HenrysLaw()
    assert henrys_law.type == mc.ReactionType.HenrysLaw
    photolysis = mc.Photolysis()
    assert photolysis.type == mc.ReactionType.Photolysis
    simpol_phase_transfer = mc.SimpolPhaseTransfer()
    assert simpol_phase_transfer.type == mc.ReactionType.SimpolPhaseTransfer
    surface = mc.Surface()
    assert surface.type == mc.ReactionType.Surface
    troe = mc.Troe()
    assert troe.type == mc.ReactionType.Troe
    ternary_chemical_activation = mc.TernaryChemicalActivation()
    assert ternary_chemical_activation.type == mc.ReactionType.TernaryChemicalActivation
    tunneling = mc.Tunneling()
    assert tunneling.type == mc.ReactionType.Tunneling
    wet_deposition = mc.WetDeposition()
    assert wet_deposition.type == mc.ReactionType.WetDeposition
    branched = mc.Branched()
    assert branched.type == mc.ReactionType.Branched
    user_defined = mc.UserDefined()
    assert user_defined.type == mc.ReactionType.UserDefined


if __name__ == "__main__":
    pytest.main([__file__])
