import pytest
import os
import shutil
from musica.mechanism_configuration import Serializer, Mechanism, Parser
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


@pytest.fixture
def temp_dir(tmp_path):
    yield tmp_path
    shutil.rmtree(tmp_path)


def test_mechanism_export(temp_dir):
    parser = Parser()
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"{temp_dir}/test_mechanism{extension}"
        MECHANISM_FULLY_DEFINED.export(path)
        mechanism = parser.parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_parser_loop(temp_dir):
    parser = Parser()
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"{temp_dir}/test_mechanism{extension}"
        Serializer.serialize(MECHANISM_FULLY_DEFINED, path)
        mechanism = parser.parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_to_file(temp_dir):
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        file_path = f'{temp_dir}/test_mechanism{extension}'
        assert not os.path.exists(file_path)
        Serializer.serialize(MECHANISM_FULLY_DEFINED, file_path)
        assert os.path.exists(file_path)


def test_bad_inputs():
    with pytest.raises(TypeError):
        Serializer.serialize(None)
    with pytest.raises(TypeError):
        Serializer.serialize('not a mechanism')


def test_path_creation(temp_dir):
    mechanism = Mechanism(name="Full Configuration")
    path = f"{temp_dir}/non_existant_path/"
    assert not os.path.exists(path)    
    Serializer.serialize(mechanism, f"{path}test_mechanism.json")
    assert os.path.exists(path)


def test_overwrite_file(temp_dir):
    mechanism = Mechanism(name="Full Configuration")
    file_path = f'{temp_dir}/test_mechanism.json'
    assert not os.path.exists(file_path)
    
    # write first file
    Serializer.serialize(mechanism, file_path)
    files = list(temp_dir.iterdir())
    assert len(files) == 1

    # overwrite file
    Serializer.serialize(mechanism, file_path)
    files = list(temp_dir.iterdir())
    assert len(files) == 1
