import pytest
import musica.mechanism_configuration as mc


def test_default_version():
    mechanism = mc.Mechanism()

    assert mechanism.version.major == 1
    assert mechanism.version.minor == 0
    assert mechanism.version.patch == 0


def test_validate():
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    gas = mc.Phase(name="gas", species=[A, B])
    reaction = mc.UserDefined(gas_phase=gas, reactants=[A], products=[B])
    mechanism = mc.Mechanism(species=[A, B], phases=[gas], reactions=[reaction])
    assert mechanism.validate() == []

    mechanism.reactions = [mc.UserDefined(gas_phase=gas, reactants=[A], products=[mc.Species(name="Z")])]
    errors = mechanism.validate()
    assert len(errors) == 1
    assert errors[0]["code"] == "ReactionRequiresUnknownSpecies"
    assert set(errors[0]) == {"code", "message", "path"}
    assert errors[0]["path"].startswith("/reactions/")


if __name__ == "__main__":
    pytest.main()
