import pytest
import musica.mechanism_configuration as mc

def test_parsed_full_v1_configuration():
  A = mc.Species(name="A")
  B = mc.Species(name="B")
  C = mc.Species(name="C")
  species = [A, B, C]
  gas = mc.Phase(name="gas", species=species)

  r1 = mc.Arrhenius(
      name="A_to_B",
      A=4.0e-3,
      C=50,
      reactants=[A],
      products=[B],
      gas_phase=gas
  )

  r2 = mc.Arrhenius(
      name="B_to_C",
      A=4.0e-3,
      C=50,  
      reactants=[B],
      products=[C],
      gas_phase=gas
  )

  mechanism = mc.Mechanism(
      name="musica_micm_example",
      species=species,
      phases=[gas],
      reactions=[r1, r2]
  )

  assert mechanism.name == "musica_micm_example"
  assert len(mechanism.species) == 3
  assert len(mechanism.phases) == 1
  assert len(mechanism.reactions) == 2
  assert mechanism.reactions[0].name == "A_to_B"
  assert mechanism.reactions[1].name == "B_to_C"
  assert mechanism.version.major == 1
  assert mechanism.version.minor == 0
  assert mechanism.version.patch == 0