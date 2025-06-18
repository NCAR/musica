from typing import Dict


def remove_empty_keys(dictionary: Dict) -> Dict:
    return {k: v for k, v in dictionary.items() if v is not None and v != "" and v != [] and v != {}}


def add_other_properties(serialize_dict: Dict, other_properties: Dict) -> None:
    for key in other_properties:
        serialize_dict[key] = other_properties[key]
