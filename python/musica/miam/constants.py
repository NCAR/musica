"""Rate and equilibrium constants for MIAM processes."""

from dataclasses import dataclass


@dataclass(frozen=True)
class HenryLawConstant:
    """Henry's Law constant with temperature dependence.

    HLC(T) = HLC_ref * exp(C * (1/T - 1/T0))   where T0 = 298.15 K

    Args:
        HLC_ref: Reference Henry's Law constant [mol m-3 Pa-1].
        C: Temperature dependence parameter [K].
    """
    HLC_ref: float
    C: float = 0.0


@dataclass(frozen=True)
class EquilibriumConstant:
    """Equilibrium constant with temperature dependence.

    K(T) = A * exp(C * (1/T0 - 1/T))   where T0 = 298.15 K

    Args:
        A: Pre-exponential factor [same units as K].
        C: Temperature dependence parameter [K].
    """
    A: float
    C: float = 0.0


@dataclass(frozen=True)
class ArrheniusRateConstant:
    """Arrhenius rate constant with temperature dependence.

    k(T) = A * exp(-C * (1/T - 1/T0))   where T0 = 298.15 K

    Args:
        A: Pre-exponential factor [appropriate units for reaction order].
        C: Activation energy parameter [K].
    """
    A: float
    C: float = 0.0
