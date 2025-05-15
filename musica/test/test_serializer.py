import pytest
from musica.mechanism_configuration import Serializer
from test_util_full_mechanism import *


def test_serialize_loop():
    # TODO: finish test
    serializer = Serializer()
    print()
    for extension in SUPPORTED_FILE_EXTENSIONS:
        path = f"examples/_missing_configuration{extension}"
        print(path)
        # serializer.serialize()
        

    # import each file
    # convent import into python object
    # test newly imported object is equal to mechanism_fully_defined
    validate_full_v1_mechanism(MECHANISM_FULLY_DEFINED) # change to mechanism from imported files


def test_serialize():
    serializer = Serializer()
    serializer.serialize(MECHANISM_FULLY_DEFINED)
    # test creates files


def test_bad_inputs():
    serializer = Serializer()
    with pytest.raises(NameError):
        serializer.serialize(None)
    with pytest.raises(TypeError):
        serializer.serialize('bad mechanism')
