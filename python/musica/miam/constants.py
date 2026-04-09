"""Rate and equilibrium constants for MIAM processes."""

from dataclasses import dataclass


@dataclass(frozen=True)
class HenrysLawConstant:
    """Henry's Law constant with temperature dependence.

    HLC(T) = hlc_ref * exp(c * (1/T - 1/T0))   where T0 = 298.15 K

    Args:
        hlc_ref: Reference Henry's Law constant [mol m-3 Pa-1].
        c: Temperature dependence parameter [K].
    """
    hlc_ref: float
    c: float = 0.0


@dataclass(frozen=True)
class EquilibriumConstant:
    """Equilibrium constant with temperature dependence.

    K(T) = a * exp(c * (1/T0 - 1/T))   where T0 = 298.15 K

    Args:
        a: Pre-exponential factor [same units as K].
        c: Temperature dependence parameter [K].
    """
    a: float
    c: float = 0.0


@dataclass(frozen=True)
class ArrheniusRateConstant:
    """Arrhenius rate constant with temperature dependence.

    k(T) = a * exp(-c * (1/T - 1/T0))   where T0 = 298.15 K

    Args:
        a: Pre-exponential factor [appropriate units for reaction order].
        c: Activation energy parameter [K].
    """
    a: float
    c: float = 0.0
