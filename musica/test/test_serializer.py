import pytest
import os
import shutil
from musica.mechanism_configuration import Serializer, Mechanism
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


# TODO:
TEST_OUTPUT_DIRECTORY = "./musica/test/test_output/"


def test_serialize_parser_loop():
    # TODO: finish test
    serializer = Serializer()
    print()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"examples/_missing_configuration{extension}"
        print(path)
        # serializer.serialize()

    # import each file
    # convent import into python object
    # test newly imported object is equal to mechanism_fully_defined
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    validate_full_v1_mechanism(MECHANISM_FULLY_DEFINED) # change to mechanism from imported files


def test_serialize_to_file():
    serializer = Serializer()
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        file_path = f'{TEST_OUTPUT_DIRECTORY}test_mechanism{extension}'
        assert not os.path.exists(file_path)
        serializer.serialize(MECHANISM_FULLY_DEFINED, file_path)
        assert os.path.exists(file_path)
    
    delete_test_output()


def test_bad_inputs():
    serializer = Serializer()
    with pytest.raises(TypeError):
        serializer.serialize(None)
    with pytest.raises(TypeError):
        serializer.serialize('not a mechanism')


def test_path_creation():
    mechanism = Mechanism(name="Full Configuration")
    serializer = Serializer()
    path = f"{TEST_OUTPUT_DIRECTORY}non_existant_path/"
    assert not os.path.exists(path)    
    serializer.serialize(mechanism, f"{path}test_mechanism.json")
    assert os.path.exists(path)

    delete_test_output()


def test_overwrite_file():
    mechanism = Mechanism(name="Full Configuration")
    serializer = Serializer()
    file_path = f'{TEST_OUTPUT_DIRECTORY}test_mechanism.json'
    assert not os.path.exists(file_path)
    
    # write first file
    serializer.serialize(mechanism, file_path)
    assert os.path.exists(file_path)
    _, _, files = next(os.walk(TEST_OUTPUT_DIRECTORY))
    assert len(files) == 1

    # overwrite file
    serializer.serialize(mechanism, file_path)
    assert os.path.exists(file_path)
    _, _, files = next(os.walk(TEST_OUTPUT_DIRECTORY))
    assert len(files) == 1

    delete_test_output()


def delete_test_output():
    if os.path.exists(TEST_OUTPUT_DIRECTORY):
        shutil.rmtree(TEST_OUTPUT_DIRECTORY)
    assert not os.path.exists(TEST_OUTPUT_DIRECTORY)
