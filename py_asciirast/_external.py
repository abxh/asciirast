#!/usr/bin/env python3

from __future__ import annotations

from ctypes import CDLL
from ctypes.util import find_library
from functools import lru_cache
from typing import Sequence, Type

import ctypes
import os

# TODO: find a better way to find the `asciirast.so` file

_libfile_path = os.path.join(".", "asciirast.so")

if not os.path.isfile(_libfile_path):
    raise Exception(
        "Ensure you have built the `.so` in the same dir as the __main__ python script."
        + "Otherwise adjust the path in _external.py."
        + "This will properly done some day."
    )

_libfile = CDLL(_libfile_path)
_libc = CDLL(find_library("c"))


@lru_cache
def typed_lib_func(
    function_name: str,
    argtypes: Sequence[Type[ctypes._CData]],
    restype: Type[ctypes._CData],
) -> ctypes._NamedFuncPointer:
    f = _libfile.__getattr__(function_name)
    f.argtypes = argtypes
    f.restype = restype
    return f


@lru_cache
def typed_libc_func(
    function_name: str,
    argtypes: Sequence[Type[ctypes._CData]],
    restype: Type[ctypes._CData],
) -> ctypes._NamedFuncPointer:
    f = _libc.__getattr__(function_name)
    f.argtypes = argtypes
    f.restype = restype
    return f
