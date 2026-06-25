import pytest
from musica.mechanism_configuration.utils import _convert_components
from musica.mechanism_configuration import ReactionComponent, Species


def test_passthrough_reaction_component():
    rc = ReactionComponent("A", 2.0)
    result = _convert_components([rc])
    assert len(result) == 1
    # An existing ReactionComponent should be returned unchanged.
    assert result[0] is rc


def test_species_becomes_component_with_default_coefficient():
    result = _convert_components([Species(name="B")])
    assert len(result) == 1
    assert isinstance(result[0], ReactionComponent)
    assert result[0].species_name == "B"
    assert result[0].coefficient == 1.0


def test_string_becomes_component_with_default_coefficient():
    result = _convert_components(["C"])
    assert len(result) == 1
    assert isinstance(result[0], ReactionComponent)
    assert result[0].species_name == "C"
    assert result[0].coefficient == 1.0


def test_species_coefficient_tuple():
    result = _convert_components([(Species(name="D"), 3.0)])
    assert result[0].species_name == "D"
    assert result[0].coefficient == 3.0


def test_coefficient_species_tuple_order_independent():
    # The coefficient may appear first in the tuple.
    result = _convert_components([(2.0, Species(name="E"))])
    assert result[0].species_name == "E"
    assert result[0].coefficient == 2.0


def test_string_coefficient_tuple():
    result = _convert_components([("F", 4.0)])
    assert result[0].species_name == "F"
    assert result[0].coefficient == 4.0


def test_coefficient_string_tuple():
    result = _convert_components([(5.0, "G")])
    assert result[0].species_name == "G"
    assert result[0].coefficient == 5.0


def test_integer_coefficient():
    result = _convert_components([(2, "H")])
    assert result[0].species_name == "H"
    assert result[0].coefficient == 2


def test_list_form_accepted():
    result = _convert_components([[Species(name="I"), 6.0]])
    assert result[0].species_name == "I"
    assert result[0].coefficient == 6.0


def test_mixed_inputs():
    items = [
        "A",
        Species(name="B"),
        (Species(name="C"), 2.0),
        (3.0, "D"),
        ReactionComponent("E", 4.0),
    ]
    result = _convert_components(items)
    assert [c.species_name for c in result] == ["A", "B", "C", "D", "E"]
    assert [c.coefficient for c in result] == [1.0, 1.0, 2.0, 3.0, 4.0]


def test_empty_input():
    assert _convert_components([]) == []


def test_invalid_input_raises_type_error():
    with pytest.raises(TypeError):
        _convert_components([42])


def test_wrong_length_tuple_raises_type_error():
    with pytest.raises(TypeError):
        _convert_components([("A", 1.0, 2.0)])
