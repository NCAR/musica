from musica.mechanism_configuration import parse
from musica.utils import find_config_path

CONFIG = find_config_path("v1", "other_properties", "config.json")


def test_parsed_other_properties_keep_type_and_structure():
    mechanism = parse(CONFIG)

    assert mechanism.other_properties == {"__nested": {"list": [1, "two", 3.5], "map": {"key": "value"}}}
    species = mechanism.species[0].other_properties
    assert species["__plain number"] == 0.5
    assert species["__quoted number"] == "0.5"
    assert species["__int"] == 3
    assert species["__flag"] is True
    assert species["__nothing"] is None
    assert mechanism.phases[0].other_properties == {"__info": {"key": "phase"}}
    reaction = mechanism.reactions.arrhenius[0]
    assert reaction.other_properties == {"__info": {"key": "reaction"}}
    assert reaction.reactants[0].other_properties == {"__tags": [1, "two"]}


def test_other_properties_survive_export(tmp_path):
    mechanism = parse(CONFIG)
    path = tmp_path / "round_trip.json"
    mechanism.export(str(path))
    reparsed = parse(str(path))

    assert reparsed.other_properties == mechanism.other_properties
    assert reparsed.species[0].other_properties == mechanism.species[0].other_properties
