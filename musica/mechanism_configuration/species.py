from typing import Optional, Any, Dict
from .. import backend
from .utils import _add_other_properties, _remove_empty_keys
from enum import Enum

_backend = backend.get_backend()
_Species = _backend._mechanism_configuration._Species


class Species(_Species):
    """
    A class representing a species in a chemical mechanism.

    Attributes:
        name (str): The name of the species.
        HLC_298K_mol_m3_Pa (float): Henry's Law Constant at 298K [mol m-3 Pa-1]
        HLC_exponential_factor_K: Henry's Law Constant exponential factor [K]
        diffusion_coefficient_m2_s (float): Diffusion coefficient [m2 s-1]
        N_star (float): A parameter used to calculate the mass accomodation factor (Ervens et al., 2003)
        molecular_weight_kg_mol (float): Molecular weight [kg mol-1]
        density_kg_m3 (float): Density [kg m-3]
        tracer_type (str): Type of tracer: 'THIRD_BODY' or None [Deprecated, use is_third_body instead]
        constant_concentration_mol_m3 (float): Constant concentration of the species (mol m-3)
        constant_mixing_ratio_mol_mol (float): Constant mixing ratio of the species (mol mol-1)
        is_third_body (bool): Whether the species is a third body.
        other_properties (Dict[str, Any]): A dictionary of other properties of the species.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        HLC_298K_mol_m3_Pa: Optional[float] = None,
        HLC_exponential_factor_K: Optional[float] = None,
        diffusion_coefficient_m2_s: Optional[float] = None,
        N_star: Optional[float] = None,
        molecular_weight_kg_mol: Optional[float] = None,
        density_kg_m3: Optional[float] = None,
        tracer_type: Optional[str] = None,  # Deprecated use is_third_body instead
        constant_concentration_mol_m3: Optional[float] = None,
        constant_mixing_ratio_mol_mol: Optional[float] = None,
        is_third_body: Optional[bool] = False,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Species object with the given parameters.

        Args:
            name (str): The name of the species.
            HLC_298K_mol_m3_Pa (float): Henry's Law Constant at 298K [mol m-3 Pa-1]
            HLC_exponential_factor_K: Henry's Law Constant exponential factor [K]
            diffusion_coefficient_m2_s (float): Diffusion coefficient [m2 s-1]
            N_star (float): A parameter used to calculate the mass accomodation factor (Ervens et al., 2003)
            molecular_weight_kg_mol (float): Molecular weight [kg mol-1]
            density_kg_m3 (float): Density [kg m-3]
            tracer_type (str): Type of tracer: 'THIRD_BODY' or None [Deprecated, use is_third_body instead]
            constant_concentration_mol_m3 (float): Constant concentration of the species (mol m-3)
            constant_mixing_ratio_mol_mol (float): Constant mixing ratio of the species (mol mol-1)
            is_third_body (bool): Whether the species is a third body.
            other_properties (Dict[str, Any]): A dictionary of other properties of the species.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.HLC_298K_mol_m3_Pa = HLC_298K_mol_m3_Pa if HLC_298K_mol_m3_Pa is not None else self.HLC_298K_mol_m3_Pa
        self.HLC_exponential_factor_K = HLC_exponential_factor_K if HLC_exponential_factor_K is not None else self.HLC_exponential_factor_K
        self.diffusion_coefficient_m2_s = diffusion_coefficient_m2_s if diffusion_coefficient_m2_s is not None else self.diffusion_coefficient_m2_s
        self.N_star = N_star if N_star is not None else self.N_star
        self.molecular_weight_kg_mol = molecular_weight_kg_mol if molecular_weight_kg_mol is not None else self.molecular_weight_kg_mol
        self.density_kg_m3 = density_kg_m3 if density_kg_m3 is not None else self.density_kg_m3
        self.tracer_type = tracer_type if tracer_type is not None else self.tracer_type
        self.constant_concentration_mol_m3 = constant_concentration_mol_m3 if constant_concentration_mol_m3 is not None else self.constant_concentration_mol_m3
        self.constant_mixing_ratio_mol_mol = constant_mixing_ratio_mol_mol if constant_mixing_ratio_mol_mol is not None else self.constant_mixing_ratio_mol_mol
        self.is_third_body = is_third_body if is_third_body is not None else self.is_third_body
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(instance) -> Dict:
        serialize_dict = {
            "name": instance.name,
            "HLC(298K) [mol m-3 Pa-1]": instance.HLC_298K_mol_m3_Pa,
            "HLC exponential factor [K]": instance.HLC_exponential_factor_K,
            "diffusion coefficient [m2 s-1]": instance.diffusion_coefficient_m2_s,
            "N star": instance.N_star,
            "molecular weight [kg mol-1]": instance.molecular_weight_kg_mol,
            "density [kg m-3]": instance.density_kg_m3,
            "tracer type": instance.tracer_type,
            "constant concentration [mol m-3]": instance.constant_concentration_mol_m3,
            "constant mixing ratio [mol mol-1]": instance.constant_mixing_ratio_mol_mol,
            "is third body": instance.is_third_body,
        }
        _add_other_properties(serialize_dict, instance.other_properties)
        return _remove_empty_keys(serialize_dict)
