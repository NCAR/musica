import pytest
import os
from musica.mechanism_configuration import Mechanism, Parser
from test_util_full_mechanism import get_fully_defined_mechanism, validate_full_v1_mechanism


def test_mechanism_export_loop(tmp_path):
    parser = Parser()
    mechanism = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        path = f"{tmp_path}/test_mechanism{extension}"
        mechanism.export(path)
        mechanism = parser.parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_parser_loop(tmp_path):
    parser = Parser()
    mechanism = get_fully_defined_mechanism()
    extensions = [".json", ".yml", ".yaml"]
    for extension in extensions:
        path = f"{tmp_path}/test_mechanism{extension}"
        mechanism.export(path)
        mechanism = parser.parse(path)
        validate_full_v1_mechanism(mechanism)


def test_serialize_to_file(tmp_path):
    mechanism = get_fully_defined_mechanism()
    extensions = [".yml", ".yaml", ".json"]
    for extension in extensions:
        file_path = f'{tmp_path}/test_mechanism{extension}'
        assert not os.path.exists(file_path)
        mechanism.export(file_path)
        assert os.path.exists(file_path)


def test_bad_inputs():
    mechanism = Mechanism(name="Full Configuration")
    with pytest.raises(Exception):
        mechanism.export("unsupported.txt")


def test_path_creation(tmp_path):
    mechanism = Mechanism(name="Full Configuration")
    path = f"{tmp_path}/non_existant_path/"
    assert not os.path.exists(path)
    mechanism.export(f"{path}test_mechanism.json")
    assert os.path.exists(path)


def test_overwrite_file(tmp_path):
    mechanism = Mechanism(name="Full Configuration")
    file_path = f'{tmp_path}/test_mechanism.json'
    assert not os.path.exists(file_path)

    # write first file
    mechanism.export(file_path)
    files = list(tmp_path.iterdir())
    assert len(files) == 1

    # overwrite file
    mechanism.export(file_path)
    files = list(tmp_path.iterdir())
    assert len(files) == 1


if __name__ == "__main__":
    pytest.main([__file__])
