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
    emission = mc.Emission()
    assert emission.type == mc.ReactionType.Emission
    first_order_loss = mc.FirstOrderLoss()
    assert first_order_loss.type == mc.ReactionType.FirstOrderLoss
    photolysis = mc.Photolysis()
    assert photolysis.type == mc.ReactionType.Photolysis
    surface = mc.Surface()
    assert surface.type == mc.ReactionType.Surface
    troe = mc.Troe()
    assert troe.type == mc.ReactionType.Troe
    ternary_chemical_activation = mc.TernaryChemicalActivation()
    assert ternary_chemical_activation.type == mc.ReactionType.TernaryChemicalActivation
    tunneling = mc.Tunneling()
    assert tunneling.type == mc.ReactionType.Tunneling
    branched = mc.Branched()
    assert branched.type == mc.ReactionType.Branched
    user_defined = mc.UserDefined()
    assert user_defined.type == mc.ReactionType.UserDefined


def test_convert_v0_to_v1():
    parser = Parser()
    base = 'configs/v0'
    configs = ['analytical', 'carbon_bond_5', 'chapman', 'robertson', 'TS1', 'surface']
    for config in configs:
        path = f"{base}/{config}/config.json"
        mechanism = parser.parse_and_convert_v0(path)
        assert mechanism is not None


if __name__ == "__main__":
    pytest.main([__file__])
