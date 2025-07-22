#!/usr/bin/env python3
"""
Test script for the refactored Arrhenius class composition pattern.

This test validates that the Arrhenius class correctly uses composition 
instead of inheritance and maintains all expected functionality.
"""

import sys
import os
import pytest

# Add the musica module to the path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

try:
    import musica.mechanism_configuration as mc
    from musica.constants import BOLTZMANN
    print("Successfully imported mechanism_configuration")
except ImportError as e:
    print(f"Import failed: {e}")
    print("This test requires the full MUSICA build environment")
    pytest.skip("MUSICA build environment not available", allow_module_level=True)


def test_arrhenius_composition():
    """Test that Arrhenius uses composition instead of inheritance."""
    # Create species and phase
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    gas = mc.Phase(name="gas", species=[A, B])
    
    # Create Arrhenius reaction
    arr = mc.Arrhenius(
        name="A->B",
        A=1.0e-12,
        B=0.5,
        C=150.0,
        D=298.0,
        E=1.0e-6,
        reactants=[A],
        products=[B],
        gas_phase=gas
    )
    
    # Test that it has the internal instance
    assert hasattr(arr, '_instance'), "Arrhenius should have _instance attribute"
    assert not isinstance(arr, mc._Arrhenius), "Arrhenius should not inherit from _Arrhenius"
    
    # Test property access
    assert arr.name == "A->B"
    assert arr.A == 1.0e-12
    assert arr.B == 0.5
    assert arr.C == 150.0
    assert arr.D == 298.0
    assert arr.E == 1.0e-6
    
    print("✓ Composition pattern working correctly")


def test_arrhenius_property_delegation():
    """Test that all properties delegate correctly to the internal instance."""
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    gas = mc.Phase(name="gas", species=[A, B])
    
    arr = mc.Arrhenius(gas_phase=gas)
    
    # Test property setting and getting
    arr.name = "test_reaction"
    assert arr.name == "test_reaction"
    
    arr.A = 2.5e-10
    assert arr.A == 2.5e-10
    
    arr.B = -1.5
    assert arr.B == -1.5
    
    arr.C = 200.0
    assert arr.C == 200.0
    
    arr.D = 273.15
    assert arr.D == 273.15
    
    arr.E = 5.0e-7
    assert arr.E == 5.0e-7
    
    print("✓ Property delegation working correctly")


def test_arrhenius_ea_conversion():
    """Test mutually exclusive C and Ea parameters with BOLTZMANN conversion."""
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    gas = mc.Phase(name="gas", species=[A, B])
    
    # Test C parameter
    arr1 = mc.Arrhenius(name="test1", C=100.0, gas_phase=gas)
    assert arr1.C == 100.0
    
    # Test Ea parameter conversion
    Ea_value = 1000.0 * BOLTZMANN  # Some activation energy
    arr2 = mc.Arrhenius(name="test2", Ea=Ea_value, gas_phase=gas)
    expected_C = -Ea_value / BOLTZMANN
    assert abs(arr2.C - expected_C) < 1e-10
    
    # Test mutual exclusivity
    try:
        arr3 = mc.Arrhenius(name="test3", C=100.0, Ea=1000.0, gas_phase=gas)
        assert False, "Should have raised ValueError for both C and Ea"
    except ValueError as e:
        assert "Cannot specify both C and Ea" in str(e)
    
    print("✓ C/Ea parameter handling working correctly")


def test_arrhenius_reactants_products():
    """Test reactants and products conversion between Python and C++ objects."""
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    C = mc.Species(name="C")
    gas = mc.Phase(name="gas", species=[A, B, C])
    
    # Test with simple species
    arr1 = mc.Arrhenius(
        name="simple", 
        reactants=[A, B], 
        products=[C],
        gas_phase=gas
    )
    
    reactants = arr1.reactants
    assert len(reactants) == 2
    assert reactants[0].name == "A"
    assert reactants[1].name == "B"
    
    products = arr1.products
    assert len(products) == 1
    assert products[0].name == "C"
    
    # Test with stoichiometric coefficients
    arr2 = mc.Arrhenius(
        name="stoich", 
        reactants=[(2.0, A)], 
        products=[(3.0, B), C],
        gas_phase=gas
    )
    
    reactants = arr2.reactants
    assert len(reactants) == 1
    assert isinstance(reactants[0], tuple)
    assert reactants[0][0] == 2.0  # coefficient
    assert reactants[0][1].name == "A"  # species
    
    products = arr2.products
    assert len(products) == 2
    # First product with coefficient
    assert isinstance(products[0], tuple)
    assert products[0][0] == 3.0
    assert products[0][1].name == "B"
    # Second product without coefficient
    assert products[1].name == "C"
    
    print("✓ Reactants and products conversion working correctly")


def test_arrhenius_serialize():
    """Test the instance-based serialize method."""
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    gas = mc.Phase(name="gas", species=[A, B])
    
    arr = mc.Arrhenius(
        name="serialize_test",
        A=1.5e-11,
        B=0.25,
        C=75.0,
        D=300.0,
        E=2.0e-6,
        reactants=[A],
        products=[B],
        gas_phase=gas,
        other_properties={"custom_prop": "test_value"}
    )
    
    # Test instance method (not static)
    serialized = arr.serialize()
    
    assert isinstance(serialized, dict)
    assert serialized["type"] == "ARRHENIUS"
    assert serialized["name"] == "serialize_test"
    assert serialized["A"] == 1.5e-11
    assert serialized["B"] == 0.25
    assert serialized["C"] == 75.0
    assert serialized["D"] == 300.0
    assert serialized["E"] == 2.0e-6
    assert serialized["gas phase"] == "gas"
    assert "reactants" in serialized
    assert "products" in serialized
    assert serialized["custom_prop"] == "test_value"
    
    print("✓ Instance-based serialize method working correctly")


def test_arrhenius_serialize_static():
    """Test the static serialize method for C++ object compatibility.""" 
    # This test would ideally check the static method with a C++ object,
    # but since we can't create _Arrhenius objects without the full build,
    # we'll just verify the method exists and can be called
    assert hasattr(mc.Arrhenius, 'serialize_static'), "Static serialize method should exist"
    print("✓ Static serialize method exists for C++ compatibility")


def test_arrhenius_gas_phase_types():
    """Test gas_phase property with both Phase objects and strings."""
    A = mc.Species(name="A")
    B = mc.Species(name="B")
    gas = mc.Phase(name="gas", species=[A, B])
    
    # Test with Phase object
    arr1 = mc.Arrhenius(name="test1", gas_phase=gas)
    assert arr1.gas_phase == "gas"
    
    # Test with string
    arr2 = mc.Arrhenius(name="test2", gas_phase="liquid")
    assert arr2.gas_phase == "liquid"
    
    # Test setting with Phase object
    arr2.gas_phase = gas
    assert arr2.gas_phase == "gas"
    
    print("✓ Gas phase type handling working correctly")


if __name__ == '__main__':
    pytest.main([__file__])