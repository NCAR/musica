import pytest
import os
import shutil
from musica.mechanism_configuration import Serializer, Mechanism
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


@pytest.fixture
def temp_dir(tmp_path):
    yield tmp_path
    shutil.rmtree(tmp_path)


@pytest.mark.skip()
def test_serialize_parser_loop(temp_dir):
    # TODO: finish test. change export key names to the correct strings.
    serializer = Serializer()
    print()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"{temp_dir}/_missing_configuration{extension}"
        print(path)
        # serializer.serialize()

    # import each file
    # convent import into python object
    # test newly imported object is equal to mechanism_fully_defined
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    validate_full_v1_mechanism(MECHANISM_FULLY_DEFINED) # change to mechanism from imported files


def test_serialize_to_file(temp_dir):
    serializer = Serializer()
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        file_path = f'{temp_dir}/test_mechanism{extension}'
        assert not os.path.exists(file_path)
        serializer.serialize(MECHANISM_FULLY_DEFINED, file_path)
        assert os.path.exists(file_path)


def test_bad_inputs():
    serializer = Serializer()
    with pytest.raises(TypeError):
        serializer.serialize(None)
    with pytest.raises(TypeError):
        serializer.serialize('not a mechanism')


def test_path_creation(temp_dir):
    mechanism = Mechanism(name="Full Configuration")
    serializer = Serializer()
    path = f"{temp_dir}/non_existant_path/"
    assert not os.path.exists(path)    
    serializer.serialize(mechanism, f"{path}test_mechanism.json")
    assert os.path.exists(path)


def test_overwrite_file(temp_dir):
    mechanism = Mechanism(name="Full Configuration")
    serializer = Serializer()
    file_path = f'{temp_dir}/test_mechanism.json'
    assert not os.path.exists(file_path)
    
    # write first file
    serializer.serialize(mechanism, file_path)
    files = list(temp_dir.iterdir())
    assert len(files) == 1

    # overwrite file
    serializer.serialize(mechanism, file_path)
    files = list(temp_dir.iterdir())
    assert len(files) == 1
