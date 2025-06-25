import pytest
from musica.mechanism_configuration import *
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
    arrhenius = Arrhenius()
    assert arrhenius.type == ReactionType.Arrhenius
    condensed_phase_arrhenius = CondensedPhaseArrhenius()
    assert condensed_phase_arrhenius.type == ReactionType.CondensedPhaseArrhenius
    condensed_phase_photolysis = CondensedPhasePhotolysis()
    assert condensed_phase_photolysis.type == ReactionType.CondensedPhasePhotolysis
    emission = Emission()
    assert emission.type == ReactionType.Emission
    first_order_loss = FirstOrderLoss()
    assert first_order_loss.type == ReactionType.FirstOrderLoss
    henrys_law = HenrysLaw()
    assert henrys_law.type == ReactionType.HenrysLaw
    photolysis = Photolysis()
    assert photolysis.type == ReactionType.Photolysis
    simpol_phase_transfer = SimpolPhaseTransfer()
    assert simpol_phase_transfer.type == ReactionType.SimpolPhaseTransfer
    surface = Surface()
    assert surface.type == ReactionType.Surface
    troe = Troe()
    assert troe.type == ReactionType.Troe
    tunneling = Tunneling()
    assert tunneling.type == ReactionType.Tunneling
    wet_deposition = WetDeposition()
    assert wet_deposition.type == ReactionType.WetDeposition
    branched = Branched()
    assert branched.type == ReactionType.Branched
    user_defined = UserDefined()
    assert user_defined.type == ReactionType.UserDefined
