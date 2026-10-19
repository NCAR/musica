import json

import musica.mechanism_configuration as mc
from musica.mechanism_configuration import parse


def _write_config(tmp_path):
    config = {
        "version": "1.0.0",
        "name": "Other properties",
        "__author": "someone",
        "__nested": {"list": ["x"], "map": {"key": "value"}},
        "species": [
            {
                "name": "A",
                "__quoted number": "0.5",
                "__plain number": 0.5,
                "__count": 3,
                "__flag": True,
                "__nothing": None,
            },
            {"name": "B"},
        ],
        "phases": [
            {
                "name": "gas",
                "species": [{"name": "A"}, {"name": "B"}],
                "__info": {"key": "phase"},
            }
        ],
        "reactions": [
            {
                "type": "ARRHENIUS",
                "gas phase": "gas",
                "reactants": [{"name": "A", "__tags": [1, "two", 3.5]}],
                "products": [{"name": "B"}],
                "__info": {"key": "reaction"},
            }
        ],
    }
    path = tmp_path / "config.json"
    path.write_text(json.dumps(config))
    return path


def test_parsed_other_properties_keep_type_and_structure(tmp_path):
    mechanism = parse(str(_write_config(tmp_path)))

    assert mechanism.other_properties == {
        "__author": "someone",
        "__nested": {"list": ["x"], "map": {"key": "value"}},
    }
    assert mechanism.species[0].other_properties == {
        "__quoted number": "0.5",
        "__plain number": 0.5,
        "__count": 3,
        "__flag": True,
        "__nothing": None,
    }
    assert mechanism.phases[0].other_properties == {"__info": {"key": "phase"}}
    reaction = mechanism.reactions.arrhenius[0]
    assert reaction.other_properties == {"__info": {"key": "reaction"}}
    assert reaction.reactants[0].other_properties == {"__tags": [1, "two", 3.5]}


def test_mechanism_other_properties_round_trip(tmp_path):
    A = mc.Species(name="A", other_properties={"__nested": {"a": [1, 2.5, "x"]}})
    gas = mc.Phase(name="gas", species=[A])
    mechanism = mc.Mechanism(
        name="round trip",
        species=[A],
        phases=[gas],
        other_properties={"__nested": {"list": ["x"]}, "__revision": 2},
    )

    serialized = mechanism.serialize()
    assert serialized["__nested"] == {"list": ["x"]}
    assert serialized["__revision"] == 2
    assert serialized["species"][0]["__nested"] == {"a": [1, 2.5, "x"]}

    path = tmp_path / "round_trip.json"
    mechanism.export(str(path))
    reparsed = parse(str(path))
    assert reparsed.other_properties == {"__nested": {"list": ["x"]}, "__revision": 2}
    assert reparsed.species[0].other_properties == {"__nested": {"a": [1, 2.5, "x"]}}
