"""Tests for severity-aware error handling in Python bindings.

This module tests the handle_error() functionality that processes MUSICA
errors based on their severity level:
- INFO (0): Silent (no exception, no warning)
- WARNING (1): Issues UserWarning via PyErr_WarnEx (or raises when warnings are errors)
- ERROR (2): Throws py::value_error → Python ValueError
- CRITICAL (3): Throws std::runtime_error → Python RuntimeError

Implementation: python/bindings/error.cpp:handle_error()
C API severity constants: include/musica/error.hpp (MUSICA_SEVERITY_*)
"""
from __future__ import annotations
import pytest
import warnings
from pathlib import Path
from musica import MICM, TUVX
from musica.utils import find_config_path


class TestErrorSeverityHandling:
    """Test error handling based on severity levels."""

    def test_error_severity_throws_value_error(self):
        with pytest.raises(ValueError) as exc_info:
            MICM(config_path="nonexistent_invalid_file.json")

        error_message = str(exc_info.value)
        assert "Error creating solver" in error_message or "config" in error_message.lower()

    def test_error_includes_context_and_cpp_message(self):
        """Test that error messages should include both a high-level (Python) and the original C++ error."""
        with pytest.raises(ValueError) as exc_info:
            MICM(config_path="definitely_does_not_exist_12345.json")

        error_message = str(exc_info.value)
        # Format: "[ Python context ]:[ C++ error details ]"
        assert error_message.startswith("Error creating solver")
        assert ":" in error_message

    def test_invalid_config_structure_throws_value_error(self):
        """Test that invalid configuration structure throws ValueError."""
        import tempfile
        import json

        # Create a temporary file with invalid JSON structure
        with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as f:
            json.dump({"invalid": "structure"}, f)
            temp_path = f.name

        try:
            with pytest.raises(ValueError) as exc_info:
                MICM(config_path=temp_path)

            error_message = str(exc_info.value)
            assert "Required key not found:" in error_message
        finally:
            Path(temp_path).unlink(missing_ok=True)

    def test_state_creation_error_has_context(self):
        """Test state creation errors have helpful context."""

        solver = MICM(config_path=find_config_path("v1", "chapman", "config.json"))

        # Invalid number of grid cells should throw python-side error.
        # Therefore the stack trace doesn't include the C++ binding layer.
        with pytest.raises(ValueError) as exc_info:
            solver.create_state(number_of_grid_cells=0)

        error_message = str(exc_info.value)
        print(error_message)
        assert "must be greater than 0" in error_message
        assert ":" not in error_message


class TestSeverityToExceptionMapping:
    """Test that error severities are correctly mapped to Python warnings/exceptions."""

    @pytest.mark.skip(reason="Need a way to trigger CRITICAL severity from Python")
    def test_critical_severity_raises_runtime_error(self):
        """Test that MUSICA_SEVERITY_CRITICAL raises RuntimeError.

        Note: Currently no Python-accessible code path triggers CRITICAL severity.
        This test documents the expected behavior and should be implemented
        when a CRITICAL error becomes accessible from Python bindings.
        """
        pass

    @pytest.mark.skip(reason="Need a way to trigger WARNING severity from Python")
    def test_warning_severity_issues_user_warning(self):
        """Test that MUSICA_SEVERITY_WARNING issues UserWarning.

        Note: Currently no Python-accessible code path triggers WARNING severity.
        This test documents the expected behavior and should be implemented
        when a WARNING becomes accessible from Python bindings.
        """
        pass

    def test_info_severity_is_silent(self):
        """Test that MUSICA_SEVERITY_INFO produces no warning or exception.

        INFO severity should be completely silent (no exception, no warning).
        Success cases use INFO severity.
        """
        _ = MICM(config_path=find_config_path("v1", "chapman", "config.json"))


if __name__ == "__main__":
    pytest.main([__file__])
