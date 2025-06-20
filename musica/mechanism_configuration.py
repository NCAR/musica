# Copyright (C) 2025 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# This file is part of the musica Python package.
# For more information, see the LICENSE file in the top-level directory of this distribution.
import os
import json
import yaml
from typing import Optional, Any, Dict, List, Union, Tuple
from musica import (
    _ReactionType,
    _Species,
    _Phase,
    _ReactionComponent,
    _Arrhenius,
    _CondensedPhaseArrhenius,
    _Troe,
    _Branched,
    _Tunneling,
    _Surface,
    _Photolysis,
    _CondensedPhasePhotolysis,
    _Emission,
    _FirstOrderLoss,
    _AqueousEquilibrium,
    _WetDeposition,
    _HenrysLaw,
    _SimpolPhaseTransfer,
    _UserDefined,
    _Reactions,
    _ReactionsIterator,
    _Mechanism,
    _Version,
    _Parser,
)

BOLTZMANN_CONSTANT_J_K = 1.380649e-23  # J K-1


class ReactionType(_ReactionType):
    """
    A enum class representing a reaction type in a chemical mechanism.
    """


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
        tracer_type (str): The type of tracer ("AEROSOL", "THIRD_BODY", "CONSTANT").
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
        tracer_type: Optional[str] = None,
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
            tracer_type (str): The type of tracer ("AEROSOL", "THIRD_BODY", "CONSTANT").
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
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "name": cls.name,
            "HLC(298K) [mol m-3 Pa-1]": cls.HLC_298K_mol_m3_Pa,
            "HLC exponential factor [K]": cls.HLC_exponential_factor_K,
            "diffusion coefficient [m2 s-1]": cls.diffusion_coefficient_m2_s,
            "N star": cls.N_star,
            "molecular weight [kg mol-1]": cls.molecular_weight_kg_mol,
            "density [kg m-3]": cls.density_kg_m3,
            "tracer type": cls.tracer_type,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Phase(_Phase):
    """
    A class representing a phase in a chemical mechanism.

    Attributes:
        name (str): The name of the phase.
        species (List[Species]): A list of species in the phase.
        other_properties (Dict[str, Any]): A dictionary of other properties of the phase.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        species: Optional[List[Species]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Phase object with the given parameters.

        Args:
            name (str): The name of the phase.
            species (List[Species]): A list of species in the phase.
            other_properties (Dict[str, Any]): A dictionary of other properties of the phase.
        """
        super().__init__()
        self.name = name
        self.species = [s.name for s in species] if species is not None else self.species
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "name": cls.name,
            "species": cls.species,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Arrhenius(_Arrhenius):
    """
    A class representing an Arrhenius rate constant.

    k = A * exp( C / T ) * ( T / D )^B * exp( 1 - E * P )

    where:
        k = rate constant
        A = pre-exponential factor [(mol m-3)^(n-1)s-1]
        B = temperature exponent [unitless]
        C = exponential term [K-1]
        D = reference temperature [K]
        E = pressure scaling term [Pa-1]
        T = temperature [K]
        P = pressure [Pa]
        n = number of reactants

    Attributes:
        name (str): The name of the Arrhenius rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1] where n is the number of reactants.
        B (float): Temperature exponent [unitless].
        C (float): Exponential term [K-1].
        D (float): Reference Temperature [K].
        E (float): Pressure scaling term [Pa-1].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Arrhenius rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        A: Optional[float] = None,
        B: Optional[float] = None,
        C: Optional[float] = None,
        Ea: Optional[float] = None,
        D: Optional[float] = None,
        E: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Arrhenius object with the given parameters.

        Args:
            name (str): The name of the Arrhenius rate constant.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1] where n is the number of reactants.
            B (float): Temperature exponent [unitless].
            C (float): Exponential term [K-1].
            Ea (float): Activation energy [J molecule-1].
            D (float): Reference Temperature [K].
            E (float): Pressure scaling term [Pa-1].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the Arrhenius rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.A = A if A is not None else self.A
        self.B = B if B is not None else self.B
        if C is not None and Ea is not None:
            raise ValueError("Cannot specify both C and Ea.")
        self.C = -Ea / BOLTZMANN_CONSTANT_J_K if Ea is not None else C if C is not None else self.C
        self.D = D if D is not None else self.D
        self.E = E if E is not None else self.E
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "ARRHENIUS",
            "name": cls.name,
            "A": cls.A,
            "B": cls.B,
            "C": cls.C,
            "D": cls.D,
            "E": cls.E,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class CondensedPhaseArrhenius(_CondensedPhaseArrhenius):
    """
    A class representing a condensed phase Arrhenius rate constant.

    Attributes:
        name (str): The name of the condensed phase Arrhenius rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
        B (float): Temperature exponent [unitless].
        C (float): Exponential term [K-1].
        Ea (float): Activation energy [J molecule-1].
        D (float): Reference Temperature [K].
        E (float): Pressure scaling term [Pa-1].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_water (Species): The water species in the aerosol phase.
        other_properties (Dict[str, Any]): A dictionary of other properties of the condensed phase Arrhenius rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        A: Optional[float] = None,
        B: Optional[float] = None,
        C: Optional[float] = None,
        Ea: Optional[float] = None,
        D: Optional[float] = None,
        E: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_water: Optional[Species] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the CondensedPhaseArrhenius object with the given parameters.

        Args:
            name (str): The name of the condensed phase Arrhenius rate constant.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
            B (float): Temperature exponent [unitless].
            C (float): Exponential term [K-1].
            Ea (float): Activation energy [J molecule-1].
            D (float): Reference Temperature [K].
            E (float): Pressure scaling term [Pa-1].
            reactants (List[Union[Species, Tuple[float, Species]]]]: A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]]: A list of products formed in the reaction.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_water (Species): The water species in the aerosol phase.
            other_properties (Dict[str, Any]): A dictionary of other properties of the condensed phase Arrhenius rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.A = A if A is not None else self.A
        self.B = B if B is not None else self.B
        if C is not None and Ea is not None:
            raise ValueError("Cannot specify both C and Ea.")
        self.C = -Ea / BOLTZMANN_CONSTANT_J_K if Ea is not None else C if C is not None else self.C
        self.D = D if D is not None else self.D
        self.E = E if E is not None else self.E
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.aerosol_phase_water = (
            aerosol_phase_water.name if aerosol_phase_water is not None else self.aerosol_phase_water
        )
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "CONDENSED_PHASE_ARRHENIUS",
            "name": cls.name,
            "A": cls.A,
            "B": cls.B,
            "C": cls.C,
            "D": cls.D,
            "E": cls.E,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products":  Serializer.serialize_list_reaction_components(cls.products),
            "aerosol phase": cls.aerosol_phase,
            "aerosol-phase water": cls.aerosol_phase_water,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Troe(_Troe):
    """
    A class representing a Troe rate constant.

    Attributes:
        name (str): The name of the Troe rate constant.
        k0_A (float): Pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1].
        k0_B (float): Temperature exponent for the low-pressure limit [unitless].
        k0_C (float): Exponential term for the low-pressure limit [K-1].
        kinf_A (float): Pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1].
        kinf_B (float): Temperature exponent for the high-pressure limit [unitless].
        kinf_C (float): Exponential term for the high-pressure limit [K-1].
        Fc (float): Troe parameter [unitless].
        N (float): Troe parameter [unitless].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Troe rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        k0_A: Optional[float] = None,
        k0_B: Optional[float] = None,
        k0_C: Optional[float] = None,
        kinf_A: Optional[float] = None,
        kinf_B: Optional[float] = None,
        kinf_C: Optional[float] = None,
        Fc: Optional[float] = None,
        N: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Troe object with the given parameters.

        k0 = k0_A * exp( k0_C / T ) * ( T / 300.0 )^k0_B
        kinf = kinf_A * exp( kinf_C / T ) * ( T / 300.0 )^kinf_B
        k = k0[M] / ( 1 + k0[M] / kinf ) * Fc^(1 + 1/N*(log10(k0[M]/kinf))^2)^-1

        where:
            k = rate constant
            k0 = low-pressure limit rate constant
            kinf = high-pressure limit rate constant
            k0_A = pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1]
            k0_B = temperature exponent for the low-pressure limit [unitless]
            k0_C = exponential term for the low-pressure limit [K-1]
            kinf_A = pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1]
            kinf_B = temperature exponent for the high-pressure limit [unitless]
            kinf_C = exponential term for the high-pressure limit [K-1]
            Fc = Troe parameter [unitless]
            N = Troe parameter [unitless]
            T = temperature [K]
            M = concentration of the third body [mol m-3]

        Args:
            name (str): The name of the Troe rate constant.
            k0_A (float): Pre-exponential factor for the low-pressure limit [(mol m-3)^(n-1)s-1].
            k0_B (float): Temperature exponent for the low-pressure limit [unitless].
            k0_C (float): Exponential term for the low-pressure limit [K-1].
            kinf_A (float): Pre-exponential factor for the high-pressure limit [(mol m-3)^(n-1)s-1].
            kinf_B (float): Temperature exponent for the high-pressure limit [unitless].
            kinf_C (float): Exponential term for the high-pressure limit [K-1].
            Fc (float): Troe parameter [unitless].
            N (float): Troe parameter [unitless].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the Troe rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.k0_A = k0_A if k0_A is not None else self.k0_A
        self.k0_B = k0_B if k0_B is not None else self.k0_B
        self.k0_C = k0_C if k0_C is not None else self.k0_C
        self.kinf_A = kinf_A if kinf_A is not None else self.kinf_A
        self.kinf_B = kinf_B if kinf_B is not None else self.kinf_B
        self.kinf_C = kinf_C if kinf_C is not None else self.kinf_C
        self.Fc = Fc if Fc is not None else self.Fc
        self.N = N if N is not None else self.N
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "TROE",
            "name": cls.name,
            "k0_A": cls.k0_A,
            "k0_B": cls.k0_B,
            "k0_C": cls.k0_C,
            "kinf_A": cls.kinf_A,
            "kinf_B": cls.kinf_B,
            "kinf_C": cls.kinf_C,
            "Fc": cls.Fc,
            "N": cls.N,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Branched(_Branched):
    """
    A class representing a branched reaction rate constant.

    (TODO: get details from MusicBox)

    Attributes:
        name (str): The name of the branched reaction rate constant.
        X (float): Pre-exponential branching factor [(mol m-3)^-(n-1)s-1].
        Y (float): Exponential branching factor [K-1].
        a0 (float): Z parameter [unitless].
        n (float): A parameter [unitless].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        nitrate_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the nitrate branch.
        alkoxy_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the alkoxy branch.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the branched reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        X: Optional[float] = None,
        Y: Optional[float] = None,
        a0: Optional[float] = None,
        n: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        nitrate_products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        alkoxy_products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Branched object with the given parameters.

        Args:
            name (str): The name of the branched reaction rate constant.
            X (float): Pre-exponential branching factor [(mol m-3)^-(n-1)s-1].
            Y (float): Exponential branching factor [K-1].
            a0 (float): Z parameter [unitless].
            n (float): A parameter [unitless].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            nitrate_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the nitrate branch.
            alkoxy_products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the alkoxy branch.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the branched reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.X = X if X is not None else self.X
        self.Y = Y if Y is not None else self.Y
        self.a0 = a0 if a0 is not None else self.a0
        self.n = n if n is not None else self.n
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.nitrate_products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in nitrate_products
            ]
            if nitrate_products is not None
            else self.nitrate_products
        )
        self.alkoxy_products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in alkoxy_products
            ]
            if alkoxy_products is not None
            else self.alkoxy_products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "BRANCHED_NO_RO2",
            "name": cls.name,
            "X": cls.X,
            "Y": cls.Y,
            "a0": cls.a0,
            "n": cls.n,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "nitrate products": Serializer.serialize_list_reaction_components(cls.nitrate_products),
            "alkoxy products": Serializer.serialize_list_reaction_components(cls.alkoxy_products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Tunneling(_Tunneling):
    """
    A class representing a quantum tunneling reaction rate constant.

    k = A * exp( -B / T ) * exp( C / T^3 )

    where:
        k = rate constant
        A = pre-exponential factor [(mol m-3)^(n-1)s-1]
        B = tunneling parameter [K^-1]
        C = tunneling parameter [K^-3]
        T = temperature [K]
        n = number of reactants

    Attributes:
        name (str): The name of the tunneling reaction rate constant.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
        B (float): Tunneling parameter [K^-1].
        C (float): Tunneling parameter [K^-3].
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the tunneling reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        A: Optional[float] = None,
        B: Optional[float] = None,
        C: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Tunneling object with the given parameters.

        Args:
            name (str): The name of the tunneling reaction rate constant.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
            B (float): Tunneling parameter [K^-1].
            C (float): Tunneling parameter [K^-3].
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the tunneling reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.A = A if A is not None else self.A
        self.B = B if B is not None else self.B
        self.C = C if C is not None else self.C
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "TUNNELING",
            "name": cls.name,
            "A": cls.A,
            "B": cls.B,
            "C": cls.C,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Surface(_Surface):
    """
    A class representing a surface in a chemical mechanism.

    (TODO: get details from MusicBox)

    Attributes:
        name (str): The name of the surface.
        reaction_probability (float): The probability of a reaction occurring on the surface.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        gas_phase_products (List[Union[Species, Tuple[float, Species]]]): The gas phase products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        reaction_probability: Optional[float] = None,
        gas_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        gas_phase_products: Optional[
            List[Union[Species, Tuple[float, Species]]]
        ] = None,
        gas_phase: Optional[Phase] = None,
        aerosol_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Surface object with the given parameters.

        Args:
            name (str): The name of the surface.
            reaction_probability (float): The probability of a reaction occurring on the surface.
            gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
            gas_phase_products (List[Union[Species, Tuple[float, Species]]]): The gas phase products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the surface.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.reaction_probability = reaction_probability if reaction_probability is not None else self.reaction_probability
        self.gas_phase_species = (
            (
                _ReactionComponent(gas_phase_species.name)
                if isinstance(gas_phase_species, Species)
                else _ReactionComponent(gas_phase_species[1].name, gas_phase_species[0])
            )
            if gas_phase_species is not None
            else self.gas_phase_species
        )
        self.gas_phase_products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in gas_phase_products
            ]
            if gas_phase_products is not None
            else self.gas_phase_products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "SURFACE",
            "name": cls.name,
            "reaction probability": cls.reaction_probability,
            "gas-phase species": cls.gas_phase_species.species_name,
            "gas-phase products": Serializer.serialize_list_reaction_components(cls.gas_phase_products),
            "gas phase": cls.gas_phase,
            "aerosol phase": cls.aerosol_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Photolysis(_Photolysis):
    """
    A class representing a photolysis reaction rate constant.

    Attributes:
        name (str): The name of the photolysis reaction rate constant.
        scaling_factor (float): The scaling factor for the photolysis rate constant.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the photolysis reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Photolysis object with the given parameters.

        Args:
            name (str): The name of the photolysis reaction rate constant.
            scaling_factor (float): The scaling factor for the photolysis rate constant.
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the photolysis reaction rate constant.
        """
        super().__init__()
        self.name = name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "PHOTOLYSIS",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class CondensedPhasePhotolysis(_CondensedPhasePhotolysis):
    """
    A class representing a condensed phase photolysis reaction rate constant.

    Attributes:
        name (str): The name of the condensed phase photolysis reaction rate constant.
        scaling_factor (float): The scaling factor for the photolysis rate constant.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_water (float): The water species in the aerosol phase [unitless].
        other_properties (Dict[str, Any]): A dictionary of other properties of the condensed phase photolysis reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_water: Optional[Species] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the CondensedPhasePhotolysis object with the given parameters.

        Args:
            name (str): The name of the condensed phase photolysis reaction rate constant.
            scaling_factor (float): The scaling factor for the photolysis rate constant.
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_water (Species): The water species in the aerosol phase [unitless].
            other_properties (Dict[str, Any]): A dictionary of other properties of the condensed phase photolysis reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.aerosol_phase_water = (
            aerosol_phase_water.name if aerosol_phase_water is not None else self.aerosol_phase_water
        )
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "CONDENSED_PHASE_PHOTOLYSIS",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "aerosol phase": cls.aerosol_phase,
            "aerosol-phase water": cls.aerosol_phase_water,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Emission(_Emission):
    """
    A class representing an emission reaction rate constant.

    Attributes:
        name (str): The name of the emission reaction rate constant.
        scaling_factor (float): The scaling factor for the emission rate constant.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the emission reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the Emission object with the given parameters.

        Args:
            name (str): The name of the emission reaction rate constant.
            scaling_factor (float): The scaling factor for the emission rate constant.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the emission reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "EMISSION",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class FirstOrderLoss(_FirstOrderLoss):
    """
    A class representing a first-order loss reaction rate constant.

    Attributes:
        name (str): The name of the first-order loss reaction rate constant.
        scaling_factor (float): The scaling factor for the first-order loss rate constant.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the first-order loss reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the FirstOrderLoss object with the given parameters.

        Args:
            name (str): The name of the first-order loss reaction rate constant.
            scaling_factor (float): The scaling factor for the first-order loss rate constant.
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the first-order loss reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "FIRST_ORDER_LOSS",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class AqueousEquilibrium(_AqueousEquilibrium):
    """
    A class representing an aqueous equilibrium reaction rate constant.

    Attributes:
        name (str): The name of the aqueous equilibrium reaction rate constant.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_water (Species): The water species in the aerosol phase.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
        C (float): Exponential term [K-1].
        k_reverse (float): Reverse rate constant [(mol m-3)^(n-1)s-1].
        other_properties (Dict[str, Any]): A dictionary of other properties of the aqueous equilibrium reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_water: Optional[Species] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        A: Optional[float] = None,
        C: Optional[float] = None,
        k_reverse: Optional[float] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the AqueousEquilibrium object with the given parameters.

        Args:
            name (str): The name of the aqueous equilibrium reaction rate constant.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_water (Species): The water species in the aerosol phase.
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            A (float): Pre-exponential factor [(mol m-3)^(n-1)s-1].
            C (float): Exponential term [K-1].
            k_reverse (float): Reverse rate constant [(mol m-3)^(n-1)s-1].
            other_properties (Dict[str, Any]): A dictionary of other properties of the aqueous equilibrium reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.aerosol_phase_water = (
            aerosol_phase_water.name if aerosol_phase_water is not None else self.aerosol_phase_water
        )
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.A = A if A is not None else self.A
        self.C = C if C is not None else self.C
        self.k_reverse = k_reverse if k_reverse is not None else self.k_reverse
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "AQUEOUS_EQUILIBRIUM",
            "name": cls.name,
            "aerosol phase": cls.aerosol_phase,
            "aerosol-phase water": cls.aerosol_phase_water,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "A": cls.A,
            "C": cls.C,
            "k_reverse": cls.k_reverse,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class WetDeposition(_WetDeposition):
    """
    A class representing a wet deposition reaction rate constant.

    Attributes:
        name (str): The name of the wet deposition reaction rate constant.
        scaling_factor (float): The scaling factor for the wet deposition rate constant.
        aerosol_phase (Phase): The aerosol phase which undergoes wet deposition.
        unknown_properties (Dict[str, Any]): A dictionary of other properties of the wet deposition reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        aerosol_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the WetDeposition object with the given parameters.

        Args:
            name (str): The name of the wet deposition reaction rate constant.
            scaling_factor (float): The scaling factor for the wet deposition rate constant.
            aerosol_phase (Phase): The aerosol phase which undergoes wet deposition.
            other_properties (Dict[str, Any]): A dictionary of other properties of the wet deposition reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "WET_DEPOSITION",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "aerosol phase": cls.aerosol_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class HenrysLaw(_HenrysLaw):
    """
    A class representing a Henry's law reaction rate constant.

    Attributes:
        name (str): The name of the Henry's law reaction rate constant.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_water (Species): The water species in the aerosol phase.
        aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
        other_properties (Dict[str, Any]): A dictionary of other properties of the Henry's law reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        gas_phase: Optional[Phase] = None,
        gas_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_water: Optional[Species] = None,
        aerosol_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the HenrysLaw object with the given parameters.

        Args:
            name (str): The name of the Henry's law reaction rate constant.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_water (Species): The water species in the aerosol phase.
            aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
            other_properties (Dict[str, Any]): A dictionary of other properties of the Henry's law reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.gas_phase_species = (
            (
                _ReactionComponent(gas_phase_species.name)
                if isinstance(gas_phase_species, Species)
                else _ReactionComponent(gas_phase_species[1].name, gas_phase_species[0])
            )
            if gas_phase_species is not None
            else self.gas_phase_species
        )
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.aerosol_phase_water = (
            aerosol_phase_water.name if aerosol_phase_water is not None else self.aerosol_phase_water
        )
        self.aerosol_phase_species = (
            (
                _ReactionComponent(aerosol_phase_species.name)
                if isinstance(aerosol_phase_species, Species)
                else _ReactionComponent(
                    aerosol_phase_species[1].name, aerosol_phase_species[0]
                )
            )
            if aerosol_phase_species is not None
            else self.aerosol_phase_species
        )
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "HL_PHASE_TRANSFER",
            "name": cls.name,
            "gas phase": cls.gas_phase,
            "gas-phase species": cls.gas_phase_species.species_name,
            "aerosol phase": cls.aerosol_phase,
            "aerosol-phase water": cls.aerosol_phase_water,
            "aerosol-phase species": cls.aerosol_phase_species.species_name,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class SimpolPhaseTransfer(_SimpolPhaseTransfer):
    """
    A class representing a simplified phase transfer reaction rate constant.

    Attributes:
        name (str): The name of the simplified phase transfer reaction rate constant.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
        aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
        aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
        B (List[float]): The B parameters [unitless].
        unknown_properties (Dict[str, Any]): A dictionary of other properties of the simplified phase transfer reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        gas_phase: Optional[Phase] = None,
        gas_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        aerosol_phase: Optional[Phase] = None,
        aerosol_phase_species: Optional[Union[Species, Tuple[float, Species]]] = None,
        B: Optional[List[float]] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the SimpolPhaseTransfer object with the given parameters.

        Args:
            name (str): The name of the simplified phase transfer reaction rate constant.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            gas_phase_species (Union[Species, Tuple[float, Species]]): The gas phase species involved in the reaction.
            aerosol_phase (Phase): The aerosol phase in which the reaction occurs.
            aerosol_phase_species (Union[Species, Tuple[float, Species]]): The aerosol phase species involved in the reaction.
            B (List[float]): The B parameters [unitless].
            other_properties (Dict[str, Any]): A dictionary of other properties of the simplified phase transfer reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.gas_phase_species = (
            (
                _ReactionComponent(gas_phase_species.name)
                if isinstance(gas_phase_species, Species)
                else _ReactionComponent(gas_phase_species[1].name, gas_phase_species[0])
            )
            if gas_phase_species is not None
            else self.gas_phase_species
        )
        self.aerosol_phase = aerosol_phase.name if aerosol_phase is not None else self.aerosol_phase
        self.aerosol_phase_species = (
            (
                _ReactionComponent(aerosol_phase_species.name)
                if isinstance(aerosol_phase_species, Species)
                else _ReactionComponent(
                    aerosol_phase_species[1].name, aerosol_phase_species[0]
                )
            )
            if aerosol_phase_species is not None
            else self.aerosol_phase_species
        )
        if B is not None:
            if len(B) != 4:
                raise ValueError("B must be a list of 4 elements.")
            self.B = B
        else:
            self.B = [0, 0, 0, 0]
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "SIMPOL_PHASE_TRANSFER",
            "name": cls.name,
            "gas phase": cls.gas_phase,
            "gas-phase species": cls.gas_phase_species.species_name,
            "aerosol phase": cls.aerosol_phase,
            "aerosol-phase species": cls.aerosol_phase_species.species_name,
            "B": cls.B,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class UserDefined(_UserDefined):
    """
    A class representing a user-defined reaction rate constant.

    Attributes:
        name (str): The name of the photolysis reaction rate constant.
        scaling_factor (float): The scaling factor for the photolysis rate constant.
        reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
        products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
        gas_phase (Phase): The gas phase in which the reaction occurs.
        other_properties (Dict[str, Any]): A dictionary of other properties of the photolysis reaction rate constant.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        scaling_factor: Optional[float] = None,
        reactants: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        products: Optional[List[Union[Species, Tuple[float, Species]]]] = None,
        gas_phase: Optional[Phase] = None,
        other_properties: Optional[Dict[str, Any]] = None,
    ):
        """
        Initializes the UserDefined object with the given parameters.

        Args:
            name (str): The name of the photolysis reaction rate constant.
            scaling_factor (float): The scaling factor for the photolysis rate constant.
            reactants (List[Union[Species, Tuple[float, Species]]]): A list of reactants involved in the reaction.
            products (List[Union[Species, Tuple[float, Species]]]): A list of products formed in the reaction.
            gas_phase (Phase): The gas phase in which the reaction occurs.
            other_properties (Dict[str, Any]): A dictionary of other properties of the photolysis reaction rate constant.
        """
        super().__init__()
        self.name = name if name is not None else self.name
        self.scaling_factor = scaling_factor if scaling_factor is not None else self.scaling_factor
        self.reactants = (
            [
                (
                    _ReactionComponent(r.name)
                    if isinstance(r, Species)
                    else _ReactionComponent(r[1].name, r[0])
                )
                for r in reactants
            ]
            if reactants is not None
            else self.reactants
        )
        self.products = (
            [
                (
                    _ReactionComponent(p.name)
                    if isinstance(p, Species)
                    else _ReactionComponent(p[1].name, p[0])
                )
                for p in products
            ]
            if products is not None
            else self.products
        )
        self.gas_phase = gas_phase.name if gas_phase is not None else self.gas_phase
        self.other_properties = other_properties if other_properties is not None else self.other_properties

    @staticmethod
    def serialize(cls):
        serialize_dict = {
            "type": "USER_DEFINED",
            "name": cls.name,
            "scaling factor": cls.scaling_factor,
            "reactants": Serializer.serialize_list_reaction_components(cls.reactants),
            "products": Serializer.serialize_list_reaction_components(cls.products),
            "gas phase": cls.gas_phase,
        }
        add_other_properties(serialize_dict, cls.other_properties)
        return remove_empty_keys(serialize_dict)


class Reactions(_Reactions):
    """
    A class representing a collection of reactions in a chemical mechanism.

    Attributes:
        reactions (List[Any]): A list of reactions in the mechanism.
    """

    def __init__(
        self,
        reactions: Optional[List[Any]] = None,
    ):
        """
        Initializes the Reactions object with the given parameters.

        Args:
            reactions (List[]): A list of reactions in the mechanism.
        """
        super().__init__(reactions)


class ReactionsIterator(_ReactionsIterator):
    """
    An iterator for the Reactions class.
    """


class Version(_Version):
    """
    A class representing the version of the mechanism.
    """


class Mechanism(_Mechanism):
    """
    A class representing a chemical mechanism.

    Attributes:
        name (str): The name of the mechanism.
        reactions (List[Reaction]): A list of reactions in the mechanism.
        species (List[Species]): A list of species in the mechanism.
        phases (List[Phase]): A list of phases in the mechanism.
        version (Version): The version of the mechanism.
    """

    def __init__(
        self,
        name: Optional[str] = None,
        reactions: Optional[List[Any]] = None,
        species: Optional[List[Species]] = None,
        phases: Optional[List[Phase]] = None,
        version: Optional[Version] = None,
    ):
        """
        Initializes the Mechanism object with the given parameters.

        Args:
            name (str): The name of the mechanism.
            reactions (List[]): A list of reactions in the mechanism.
            species (List[Species]): A list of species in the mechanism.
            phases (List[Phase]): A list of phases in the mechanism.
            version (Version): The version of the mechanism.
        """
        super().__init__()
        self.name = name
        self.species = species if species is not None else []
        self.phases = phases if phases is not None else []
        self.reactions = Reactions(reactions=reactions if reactions is not None else [])
        self.version = version if version is not None else Version()

    def to_dict(self):
        species_list = []
        for species in self.species:
            species_list.append(Species.serialize(species))

        phases_list = []
        for phase in self.phases:
            phases_list.append(Phase.serialize(phase))

        reactions_list = []
        for reaction in self.reactions:
            if type(reaction) == type(_Arrhenius()) or type(reaction) == type(Arrhenius()):
                reactions_list.append(Arrhenius.serialize(reaction))
            elif type(reaction) == type(_Branched()) or type(reaction) == type(Branched()):
                reactions_list.append(Branched.serialize(reaction))
            elif type(reaction) == type(_CondensedPhaseArrhenius()) or type(reaction) == type(CondensedPhaseArrhenius()):
                reactions_list.append(CondensedPhaseArrhenius.serialize(reaction))
            elif type(reaction) == type(_CondensedPhasePhotolysis()) or type(reaction) == type(CondensedPhasePhotolysis()):
                reactions_list.append(CondensedPhasePhotolysis.serialize(reaction))
            elif type(reaction) == type(_Emission()) or type(reaction) == type(Emission()):
                reactions_list.append(Emission.serialize(reaction))
            elif type(reaction) == type(_FirstOrderLoss()) or type(reaction) == type(FirstOrderLoss()):
                reactions_list.append(FirstOrderLoss.serialize(reaction))
            elif type(reaction) == type(_SimpolPhaseTransfer()) or type(reaction) == type(SimpolPhaseTransfer()):
                reactions_list.append(SimpolPhaseTransfer.serialize(reaction))
            elif type(reaction) == type(_AqueousEquilibrium()) or type(reaction) == type(AqueousEquilibrium()):
                reactions_list.append(AqueousEquilibrium.serialize(reaction))
            elif type(reaction) == type(_WetDeposition()) or type(reaction) == type(WetDeposition()):
                reactions_list.append(WetDeposition.serialize(reaction))
            elif type(reaction) == type(_HenrysLaw()) or type(reaction) == type(HenrysLaw()):
                reactions_list.append(HenrysLaw.serialize(reaction))
            elif type(reaction) == type(_Photolysis()) or type(reaction) == type(Photolysis()):
                reactions_list.append(Photolysis.serialize(reaction))
            elif type(reaction) == type(_Surface()) or type(reaction) == type(Surface()):
                reactions_list.append(Surface.serialize(reaction))
            elif type(reaction) == type(_Troe()) or type(reaction) == type(Troe()):
                reactions_list.append(Troe.serialize(reaction))
            elif type(reaction) == type(_Tunneling()) or type(reaction) == type(Tunneling()):
                reactions_list.append(Tunneling.serialize(reaction))
            elif type(reaction) == type(_UserDefined()) or type(reaction) == type(UserDefined()):
                reactions_list.append(UserDefined.serialize(reaction))
            else:
                raise TypeError(f'Reaction type {type(reaction)} is not supported for export.')

        return {
            "name": self.name,
            "reactions": reactions_list,
            "species": species_list,
            "phases": phases_list,
            "version": self.version.to_string(),
        }

    def export(self, file_path):
        Serializer.serialize(self, file_path)


class Parser(_Parser):
    """
    A class for parsing a chemical mechanism.
    """


class Serializer():
    """
    A class for exporting a chemical mechanism.
    """

    @staticmethod
    def serialize(mechanism: Mechanism, file_path: str = "./mechanism.json"):        
        if not isinstance(mechanism, Mechanism):
            raise TypeError(f"Object {mechanism} is not of type Mechanism.")

        directory, file = os.path.split(file_path)
        if directory:
            os.makedirs(directory, exist_ok=True)
        dictionary = mechanism.to_dict()
        
        _, file_ext = os.path.splitext(file)
        if file_ext in ['.yaml', '.yml']:
            with open(file_path, 'w') as file:
                yaml.dump(dictionary, file)
        elif '.json' == file_ext:
            json_str = json.dumps(dictionary, indent=4)
            with open(file_path, 'w') as file:
                file.write(json_str)
        else:
            raise Exception('Allowable write formats are .json and .yaml')
    
    @staticmethod
    def serialize_reaction_component(rc):
        if isinstance(rc, Species) or isinstance(rc, _Species):
            return rc.name

        return remove_empty_keys({
            "species name": rc.species_name,
            "coefficient": rc.coefficient,
            "other_properties": rc.other_properties,
        })

    @staticmethod
    def serialize_list_reaction_components(reaction_component_list):
        ret = []
        for rc in reaction_component_list:
            ret.append(Serializer.serialize_reaction_component(rc))
        return ret


def remove_empty_keys(dictionary):
    return {k: v for k, v in dictionary.items() if v is not None and v != "" and v != [] and v != {}}


def add_other_properties(serialize_dict, other_properties):
    for key in other_properties:
        serialize_dict[key] = other_properties[key]
