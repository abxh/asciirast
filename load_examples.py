#!/usr/bin/env python3

import os
import examples
import py_asciirast as asciirast


def iter_submodules(module):
    import pkgutil
    import importlib

    for _, module_name, _ in pkgutil.walk_packages(
        module.__path__, module.__name__ + "."
    ):
        yield importlib.import_module(module_name)


if is_windows := os.name == "nt":
    from colorama import just_fix_windows_console

    just_fix_windows_console()

if not os.path.exists(asciirast.get_libfile_path()):
    print(f"Cannot file {asciirast.get_libfile_path()}. Please specify file path:")
    asciirast.set_libfile_path(input())

    if not os.path.exists(asciirast.get_libfile_path()):
        raise Exception(
            "Cannot find file. Try building the library before running this script."
        )


l = list(iter_submodules(examples))

for i, module in enumerate(l):
    print(f"{i} : {module.__name__.split('.')[1]}")

if not (inp := input()).isdigit():
    print(f"{inp} is not a integer.")
elif not 0 <= int(inp) < len(l):
    print(f"{inp} is not a valid index!")
else:
    l[int(inp)].main()
