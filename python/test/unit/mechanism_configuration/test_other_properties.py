import json

import musica.mechanism_configuration as mc
from musica.mechanism_configuration import parse


def _write_config(tmp_path):
    config = {
        "version": "1.0.0",
        "name": "Other properties",
        "__author": "someone",
        "__cam_chem": {"fixed": ["CH4"], "ext_forcing": {"SO2": "Forced"}},
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
                "__kpp": {"integrator": "rosenbrock"},
            }
        ],
        "reactions": [
            {
                "type": "ARRHENIUS",
                "gas phase": "gas",
                "reactants": [{"name": "A", "__tags": [1, "two", 3.5]}],
                "products": [{"name": "B"}],
                "__wrf_chem": {"custom_rate": "usr_CO_OH_a"},
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
        "__cam_chem": {"fixed": ["CH4"], "ext_forcing": {"SO2": "Forced"}},
    }
    assert mechanism.species[0].other_properties == {
        "__quoted number": "0.5",
        "__plain number": 0.5,
        "__count": 3,
        "__flag": True,
        "__nothing": None,
    }
    assert mechanism.phases[0].other_properties == {"__kpp": {"integrator": "rosenbrock"}}
    reaction = mechanism.reactions.arrhenius[0]
    assert reaction.other_properties == {"__wrf_chem": {"custom_rate": "usr_CO_OH_a"}}
    assert reaction.reactants[0].other_properties == {"__tags": [1, "two", 3.5]}


def test_mechanism_other_properties_round_trip(tmp_path):
    A = mc.Species(name="A", other_properties={"__nested": {"a": [1, 2.5, "x"]}})
    gas = mc.Phase(name="gas", species=[A])
    mechanism = mc.Mechanism(
        name="round trip",
        species=[A],
        phases=[gas],
        other_properties={"__cam_chem": {"fixed": ["CH4"]}, "__revision": 2},
    )

    serialized = mechanism.serialize()
    assert serialized["__cam_chem"] == {"fixed": ["CH4"]}
    assert serialized["__revision"] == 2
    assert serialized["species"][0]["__nested"] == {"a": [1, 2.5, "x"]}

    path = tmp_path / "round_trip.json"
    mechanism.export(str(path))
    reparsed = parse(str(path))
    assert reparsed.other_properties == {"__cam_chem": {"fixed": ["CH4"]}, "__revision": 2}
    assert reparsed.species[0].other_properties == {"__nested": {"a": [1, 2.5, "x"]}}
