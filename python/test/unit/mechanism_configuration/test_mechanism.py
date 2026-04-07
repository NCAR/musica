import pytest
import musica.mechanism_configuration as mc

def test_default_version():
  mechanism = mc.Mechanism()

  assert mechanism.version.major == 1
  assert mechanism.version.minor == 0
  assert mechanism.version.patch == 0

if __name__ == "__main__":
  pytest.main()