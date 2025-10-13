"""MUSICA Examples Module.

This module provides a centralized way to access and manage MUSICA examples.
It defines the Example class for representing individual examples and the
Examples singleton for accessing all available examples.

Example:
    >>> from musica.examples import Examples
    >>> print(Examples.TS1LatinHyperCube)
    >>> for example in Examples:
    ...     print(example.name)
"""


class Example:
    """A class representing a MUSICA example.

    This class encapsulates information about a specific example, including its name,
    short name, description, and file path. It provides methods for string representation
    and class-based construction.
    """

    def __init__(self, name, short_name, description, path):
        """Initialize an Example instance.

        Args:
            name (str): The display name of the example.
            short_name (str): A short identifier for the example.
            description (str): A detailed description of what the example demonstrates.
            path (str): The file path to the example script.
        """
        self.name = name
        self.short_name = short_name
        self.description = description
        self.path = path

    def __str__(self):
        """Return a string representation of the example.

        Returns:
            str: A formatted string showing the name and description.
        """
        return f'{self.name}: {self.description}'

    def __repr__(self):
        """Return a string representation of the example for debugging.

        Returns:
            str: A formatted string showing the name and description.
        """
        return f'{self.name}: {self.description}'

    @classmethod
    def from_config(cls, display_name, path, short_name, description):
        """Create an Example instance from configuration parameters.

        This class method provides an alternative constructor that creates an Example
        instance with more explicit parameter naming.

        Args:
            display_name (str): The display name of the example.
            path (str): The file path to the example script.
            short_name (str): A short identifier for the example.
            description (str): A detailed description of what the example demonstrates.

        Returns:
            Example: A new Example instance with the provided configuration.
        """
        return cls(name=display_name, short_name=short_name, description=description, path=path)


class _Examples:
    """A container class for managing MUSICA examples.

    This class provides a centralized way to access and manage all available MUSICA
    examples. It includes predefined examples and supports iteration, indexing, and
    attribute access patterns.
    """
    CARMA_Aluminum = Example.from_config(
        display_name='CARMA Aluminum',
        short_name='CARMA_Aluminum',
        path='carma_aluminum.py',
        description='A CARMA example for simulating aluminum aerosol particles.')
    CARMA_Sulfate = Example.from_config(
        display_name='CARMA Sulfate',
        short_name='CARMA_Sulfate',
        path='carma_sulfate.py',
        description='A CARMA example for simulating sulfate aerosol particles.')
    Sulfate_Box_Model = Example.from_config(
        display_name='Sulfate Box Model',
        short_name='Sulfate_Box_Model',
        path='sulfate_box_model.py',
        description='A box model example for simulating sulfate aerosol particles.')
    TS1LatinHyperCube = Example.from_config(
        display_name='TS1 Latin Hypercube',
        short_name='TS1LatinHyperCube',
        path='ts1_latin_hypercube.py',
        description='A Latin hypercube sampling example for the TS1 mechanism. This script shows how to sample an input space and run multiple box models in parallel on a single mechanism.')

    @classmethod
    def get_all(cls):
        """Get all available examples.

        Returns:
            list[Example]: A list of all available Example instances.
        """
        return [cls.CARMA_Aluminum, cls.CARMA_Sulfate, cls.Sulfate_Box_Model, cls.TS1LatinHyperCube]

    def __iter__(self):
        """Make the class iterable over examples.

        Returns:
            iterator: An iterator over all available examples.
        """
        return iter(self.get_all())

    def __getattr__(self, item):
        """Handle attribute access for examples.

        Args:
            item (str): The attribute name to access.

        Returns:
            Any: The requested attribute value.

        Raises:
            AttributeError: If the attribute doesn't exist.
        """
        # Check if the attribute exists in the class definition
        if hasattr(self.__class__, item):
            return getattr(self.__class__, item)
        raise AttributeError(f"'{self.__class__.__name__}' object has no attribute '{item}'")

    def __getitem__(self, item):
        """Support indexing to access examples by position.

        Args:
            item (int): The index of the example to retrieve.

        Returns:
            Example: The example at the specified index.

        Raises:
            IndexError: If the index is out of range.
        """
        return self.get_all()[item]

    def __repr__(self):
        """Return a string representation for debugging.

        Returns:
            str: A formatted string showing all available examples.
        """
        return f'Examples: {self.get_all()}'

    def __str__(self):
        """Return a string representation of the examples collection.

        Returns:
            str: A formatted string showing all available examples.
        """
        return f'Examples: {self.get_all()}'


Examples = _Examples()
