import os
import sys


def pytest_sessionfinish(session, exitstatus):
    """Work around a Windows process-teardown deadlock in tuvx-linked native code.

    On Windows, tearing down the loaded native libraries (the tuvx/gfortran
    runtime and its dependencies) during interpreter shutdown deadlocks under
    the loader lock, so the process hangs *after* all tests have finished and
    the summary has been printed. The same deadlock shows up in the standalone
    C++ gtest binaries (ctest survives only because it kills them on timeout).

    Once the session is finished the test results are already reported, so we
    don't need a clean interpreter shutdown. os._exit() terminates immediately,
    bypassing atexit handlers and DLL teardown, while preserving the exit code.
    """
    if sys.platform == "win32":
        sys.stdout.flush()
        sys.stderr.flush()
        os._exit(int(exitstatus))
