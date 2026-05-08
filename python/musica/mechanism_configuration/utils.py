from typing import Dict


def _remove_empty_keys(dictionary: Dict) -> Dict:
    return {k: v for k, v in dictionary.items() if v is not None and v != "" and v != {}}


def _add_other_properties(serialize_dict: Dict, other_properties: Dict) -> None:
    for key in other_properties:
        serialize_dict[key] = other_properties[key]


def _convert_components(items):
    """Convert List[Union[Species, Tuple[Species, float], ReactionComponent]] to List[ReactionComponent].

    Import is deferred to avoid circular imports.
    """
    from .reaction_component import ReactionComponent
    from .species import Species
    return [
        (
            component
            if isinstance(component, ReactionComponent)
            else ReactionComponent(component.name)
            if isinstance(component, Species)
            else ReactionComponent(component[0].name, component[1])
        )
        for component in items
    ]


def _format_component(component) -> str:
    coef = component.coefficient
    name = component.species_name
    if coef == 1.0:
        return name
    coef_str = str(int(coef)) if coef == int(coef) else str(coef)
    return f"{coef_str}{name}"


def _format_components(components) -> str:
    return " + ".join(_format_component(c) for c in components)
