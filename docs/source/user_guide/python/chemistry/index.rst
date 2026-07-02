.. _chemistry:

Defining chemical systems
=========================

This section covers the components of a chemical system as defined in MUSICA:

- Species (see :class:`musica.mechanism_configuration.species.Species`)
- Phases (see :class:`musica.mechanism_configuration.phase.Phase`)
- Reactions (see :class:`musica.mechanism_configuration.reactions.Reactions`)
- Mechanisms (see :class:`musica.mechanism_configuration.Mechanism`)


As a reminder, this section assumes you have imported::
   
   import musica.mechanism_configuration as mc

Species
--------
Chemical species are the fundamental units that participate in reactions, they can be defined using::
   
   X = mc.Species(name="X")
   Y = mc.Species(name="Y")
   Z = mc.Species(name="Z")
   species = {"X": X, "Y": Y, "Z": Z}

Species can be initialized with various chemistry-related parameters (e.g., molecular weight, density, etc.). See the :class:`musica.mechanism_configuration.species.Species`
documentation for further details.

Phases
-------
Species can be grouped into a phase. At present MUSICA and therefore MusicBox can only support a single gas phase,
but support for more phases is coming in the future::

   gas = mc.Phase(name="gas", species=list(species.values()))

Reactions
----------
Reactions are defined using rate-based classes such as :class:`musica.mechanism_configuration.arrhenius.Arrhenius`.
Each class takes a unique set of rate parameters and the participating species::

   arr1 = mc.Arrhenius(name="X->Y", A=4.0e-3, C=50, reactants=[species["X"]], products=[species["Y"]], gas_phase=gas)
   arr2 = mc.Arrhenius(name="Y->Z", A=4.0e-3, C=50, reactants=[species["Y"]], products=[species["Z"]], gas_phase=gas)
   rxns = {"X->Y": arr1, "Y->Z": arr2}

.. note::

   Several other reaction types are made available for use in MusicBox. For a full list of supported reactions types as well as their parameters,
   please see the :ref:`mc:reactions` page in the OpenAtmos Mechanism Configuration documentation.

Mechanisms
----------
A mechanism defines a collection of chemical species, their associated phases, and the reactions between them. Mechanisms can be defined as
follows::

   mechanism = mc.Mechanism(name="musica_example", species=list(species.values()), phases=[gas], reactions=list(rxns.values()))
