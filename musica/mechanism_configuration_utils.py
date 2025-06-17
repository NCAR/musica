def remove_empty_keys(dictionary):
    return {k: v for k, v in dictionary.items() if v is not None and v != "" and v != [] and v != {}}


def add_other_properties(serialize_dict, other_properties):
    for key in other_properties:
        serialize_dict[key] = other_properties[key]
