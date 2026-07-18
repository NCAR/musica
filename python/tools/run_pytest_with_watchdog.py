#!/usr/bin/env python
# Copyright (C) 2026 University Corporation for Atmospheric Research
# SPDX-License-Identifier: Apache-2.0
#
# Runs pytest with a faulthandler watchdog so a CI hang (e.g. the PR #960
# windows-11-arm stall, where the test process never exited after printing
# its final summary) produces a stack trace of every live thread instead of
# silently consuming the whole job timeout.
import faulthandler
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
    sys.exit(main())
