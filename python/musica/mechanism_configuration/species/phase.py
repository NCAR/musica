from typing import Optional, Any, Dict, List, Union
from ... import backend
from ..._base import CppWrapper, CppField, _unwrap_list, _wrap_list
from .species import Species
from .phase_species import PhaseSpecies
from ..utils import _add_other_properties, _remove_empty_keys

_backend = backend.get_backend()
_mc = _backend._mechanism_configuration


class Phase(CppWrapper):
    """A phase in a chemical mechanism.

    Attributes:
        name: The name of the phase.
        species: A list of species in the phase.
        other_properties: A dictionary of other properties of the phase.
    """

    name = CppField()
    other_properties = CppField()

    def __init__(
        self,
        name: Optional[str] = None,
        species: Optional[List[Union[Species, PhaseSpecies]]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """Initialize the Phase.

        Args:
            name: The name of the phase.
            species: A list of species in the phase. Species objects are
                     automatically converted to PhaseSpecies.
            other_properties: A dictionary of other properties of the phase.
        """
        self._cpp = _mc._Phase()
        self.name = name if name is not None else self.name
        converted_species = []
        if species is not None:
            for s in species:
                if isinstance(s, PhaseSpecies):
                    converted_species.append(s)
                elif isinstance(s, Species):
                    converted_species.append(PhaseSpecies(species=s))
        self.species = converted_species
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @property
    def species(self) -> List[PhaseSpecies]:
        """List of species in the phase."""
        return _wrap_list(PhaseSpecies, self._cpp.species)

    @species.setter
    def species(self, value: List[PhaseSpecies]):
        self._cpp.species = _unwrap_list(value)

    def add(
        self,
        species: Union[Species, PhaseSpecies, str],
        *,
        diffusion_coefficient_m2_s: Optional[float] = None,
        density_kg_m3: Optional[float] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ) -> "Phase":
        """Add a species to the phase, with optional phase-specific properties.

        Users do not need to build a PhaseSpecies directly.

        Args:
            species: A Species, or the name of a species. A PhaseSpecies is
                     added as-is and must not be combined with properties.
            diffusion_coefficient_m2_s: Diffusion coefficient in the phase [m2 s-1].
            density_kg_m3: Density in the phase [kg m-3].
            other_properties: A dictionary of other phase-specific properties.

        Returns:
            This phase, so calls can be chained.
        """
        has_properties = (diffusion_coefficient_m2_s is not None
                          or density_kg_m3 is not None
                          or other_properties is not None)
        if isinstance(species, PhaseSpecies):
            if has_properties:
                raise ValueError("Properties cannot be given together with a PhaseSpecies")
            new_species = species
        else:
            kwargs = dict(
                diffusion_coefficient_m2_s=diffusion_coefficient_m2_s,
                density_kg_m3=density_kg_m3,
                other_properties=other_properties,
            )
            if isinstance(species, Species):
                new_species = PhaseSpecies(species=species, **kwargs)
            elif isinstance(species, str):
                new_species = PhaseSpecies(name=species, **kwargs)
            else:
                raise TypeError(f"Cannot add {type(species).__name__} to a Phase")
        self.species = self.species + [new_species]
        return self

    def serialize(self):
        serialize_dict = {
            "name": self.name,
            "species": [s.serialize() for s in self.species],
        }
        _add_other_properties(serialize_dict, self.other_properties)
        return _remove_empty_keys(serialize_dict)
