import pytest
import os
from musica.mechanism_configuration import MechanismSerializer, Mechanism, Parser
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


def test_mechanism_export_loop(tmp_path):
    parser = Parser()
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"{tmp_path}/test_mechanism{extension}"
        MECHANISM_FULLY_DEFINED.export(path)
        mechanism = parser.parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_parser_loop(tmp_path):
    parser = Parser()
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"{tmp_path}/test_mechanism{extension}"
        MechanismSerializer.serialize(MECHANISM_FULLY_DEFINED, path)
        mechanism = parser.parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_to_file(tmp_path):
    MECHANISM_FULLY_DEFINED = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        file_path = f'{tmp_path}/test_mechanism{extension}'
        assert not os.path.exists(file_path)
        MechanismSerializer.serialize(MECHANISM_FULLY_DEFINED, file_path)
        assert os.path.exists(file_path)


def test_bad_inputs():
    with pytest.raises(TypeError):
        MechanismSerializer.serialize(None)
    with pytest.raises(TypeError):
        MechanismSerializer.serialize('not a mechanism')
    with pytest.raises(Exception):
        MechanismSerializer.serialize(get_fully_defined_mechanism(), 'unsupported.txt')


def test_path_creation(tmp_path):
    mechanism = Mechanism(name="Full Configuration")
    path = f"{tmp_path}/non_existant_path/"
    assert not os.path.exists(path)
    MechanismSerializer.serialize(mechanism, f"{path}test_mechanism.json")
    assert os.path.exists(path)


def test_overwrite_file(tmp_path):
    mechanism = Mechanism(name="Full Configuration")
    file_path = f'{tmp_path}/test_mechanism.json'
    assert not os.path.exists(file_path)

    # write first file
    MechanismSerializer.serialize(mechanism, file_path)
    files = list(tmp_path.iterdir())
    assert len(files) == 1

    # overwrite file
    MechanismSerializer.serialize(mechanism, file_path)
    files = list(tmp_path.iterdir())
    assert len(files) == 1
