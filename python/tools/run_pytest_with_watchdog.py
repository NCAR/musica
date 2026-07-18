#!/usr/bin/env python
# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Runs pytest with a faulthandler watchdog so a CI hang (e.g. the PR #960
# windows-11-arm stall, where the test process never exited after printing
# its final summary) produces a stack trace of every live thread instead of
# silently consuming the whole job timeout.
#
# Once pytest itself has finished, we skip the normal interpreter shutdown
# path entirely. On Windows this process has hung for up to 90 minutes
# *after* printing its results and confirming no non-main Python threads
# remain, meaning the hang is inside DLL_PROCESS_DETACH teardown of the
# compiled extension (native static destructors), which faulthandler can't
# see since it's torn down before that point. os._exit() alone did NOT fix
# this on Windows: the CRT's _exit() still goes through ExitProcess(), which
# itself still delivers DLL_PROCESS_DETACH to every loaded DLL before the
# process actually dies. TerminateProcess() is the only Win32 call that
# skips DLL_PROCESS_DETACH notification entirely, so on Windows we call it
# directly via ctypes. Since all tests have already passed by this point, a
# clean interpreter/DLL shutdown isn't needed here.
import faulthandler
import os
import sys
import threading

import pytest

# The suite normally runs in well under a minute; this only fires on a hang.
WATCHDOG_SECONDS = 600


def main() -> int:
    faulthandler.dump_traceback_later(WATCHDOG_SECONDS, exit=True)

    exit_code = pytest.main(sys.argv[1:])

    print(f"pytest.main() returned {exit_code}", flush=True)
    live = [t.name for t in threading.enumerate() if t is not threading.main_thread()]
    print(f"non-main threads still alive: {live or 'none'}", flush=True)

    return exit_code


def hard_exit(code: int) -> None:
    code = code or 0
    if sys.platform == "win32":
        import ctypes

        kernel32 = ctypes.windll.kernel32
        kernel32.TerminateProcess(kernel32.GetCurrentProcess(), ctypes.c_uint(code))
    else:
        os._exit(code)


if __name__ == "__main__":
    hard_exit(main())
