import pytest
import musica.mechanism_configuration as mc


def test_add_species_with_properties():
    h2o = mc.Species(name="H2O")
    phase = mc.Phase(name="AQUEOUS")
    result = phase.add(h2o, density_kg_m3=1000.0, diffusion_coefficient_m2_s=1e-9)
    assert result is phase
    assert len(phase.species) == 1
    assert phase.species[0].name == "H2O"
    assert phase.species[0].density_kg_m3 == 1000.0
    assert phase.species[0].diffusion_coefficient_m2_s == 1e-9


def test_add_by_name_and_chain():
    phase = mc.Phase(name="gas").add("A").add(mc.Species(name="B"))
    assert [s.name for s in phase.species] == ["A", "B"]


def test_add_phase_species_with_properties_raises():
    with pytest.raises(ValueError):
        mc.Phase(name="gas").add(mc.PhaseSpecies(name="A"), density_kg_m3=1.0)


def test_add_bad_type_raises():
    with pytest.raises(TypeError):
        mc.Phase(name="gas").add(3)
