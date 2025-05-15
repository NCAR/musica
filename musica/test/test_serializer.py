import pytest
from musica.mechanism_configuration import Serializer
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


def test_serialize_loop():
    # TODO: finish test
    serializer = Serializer()
    print()
    extensions = [".yaml", ".json"]
    for extension in extensions:
        path = f"examples/_missing_configuration{extension}"
        print(path)
        # serializer.serialize()

    # import each file
    # convent import into python object
    # test newly imported object is equal to mechanism_fully_defined
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    validate_full_v1_mechanism(MECHANISM_FULLY_DEFINED) # change to mechanism from imported files


def test_serialize():
    serializer = Serializer()
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    serializer.serialize(MECHANISM_FULLY_DEFINED)
    # test creates files


def test_bad_inputs():
    serializer = Serializer()
    with pytest.raises(NameError):
        serializer.serialize(None)
    with pytest.raises(TypeError):
        serializer.serialize('bad mechanism')
