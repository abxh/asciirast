#!/usr/bin/env python3

import os

if is_windows := os.name == "nt":
    from colorama import just_fix_windows_console

    just_fix_windows_console()

from examples import spiral

spiral.main()
