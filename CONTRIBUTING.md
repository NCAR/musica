# Contributing to MUSICA

We welcome contributions from the community! This guide outlines how you can contribute to the MUSICA project.

## Getting Started

### Development Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/NCAR/musica.git
   cd musica
   ```

2. **Build the project:**
   ```bash
   mkdir build && cd build
   ccmake ..
   make
   ```

3. **Python development setup:**
   ```bash
   pip install -e .
   ```

For detailed developer options and dependency management, see our [Software Development Plan](docs/Software%20Development%20Plan.pdf).

## Types of Contributions

We appreciate all types of contributions:

### Code Contributions
- Bug fixes
- New features
- Performance improvements
- Test coverage improvements

### Documentation
- README improvements
- Code documentation
- Tutorial and example updates
- Wiki contributions

### Testing and Validation
- Bug reports with reproducible examples
- Testing on different platforms
- Validation against known results

### Scientific Contributions
- New chemical mechanisms
- Algorithm improvements
- Performance optimizations

## Contribution Process

1. **Fork the repository** and create a feature branch
2. **Make your changes** following our coding standards
3. **Add tests** for new functionality
4. **Update documentation** as needed
5. **Submit a pull request** with a clear description

## Recognition Policy

We believe in recognizing all contributors appropriately:

### Core Development Team
Contributors who make substantial, ongoing contributions to the codebase, architecture, or project direction will be listed as authors in:
- `pyproject.toml` (for Python package metadata)
- `.zenodo.json` (as "creators" for software citations)
- `AUTHORS.md` (as core developers)

### Additional Contributors  
Contributors who make valuable but smaller contributions will be acknowledged in:
- `.zenodo.json` (as "contributors" with appropriate type)
- `AUTHORS.md` (in the Additional Contributors section)
- GitHub's contributor list (automatic)

## Development Guidelines

### Code Standards
- Follow existing code style and conventions
- Include appropriate tests for new functionality
- Document new features and APIs

### Dependency Management
We use CMake for C++ dependencies and pip for Python dependencies. See our [README](README.md#developer-options) for information about specifying dependency versions during development.

### GPU Support
If contributing GPU-related code, please test on appropriate hardware and follow our GPU build guidelines.

## Questions?

- Check our [documentation](https://ncar.github.io/musica/index.html)
- Read our [Software Development Plan](docs/Software%20Development%20Plan.pdf)
- Contact the maintainers at musica-support@ucar.edu

Thank you for your interest in contributing to MUSICA!
