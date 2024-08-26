#!/usr/bin/env python3

from __future__ import annotations

from ctypes import CDLL
from ctypes.util import find_library
from functools import lru_cache
from typing import Sequence, Type

import ctypes
import os

_libfile = CDLL(os.path.join(".", "rasterizer.so"))
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
