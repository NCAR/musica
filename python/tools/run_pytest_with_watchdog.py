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
# path (os._exit instead of sys.exit/return). On Windows this process has
# hung for up to 90 minutes *after* printing its results and confirming no
# non-main Python threads remain, meaning the hang is inside CPython's own
# finalization (extension-module DLL unload / native static destructors),
# which runs after faulthandler itself has already been torn down -- so the
# watchdog above can never see or diagnose it. Since all tests have already
# passed by this point, a clean interpreter shutdown isn't needed here.
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


if __name__ == "__main__":
    os._exit(main())
