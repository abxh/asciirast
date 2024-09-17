#!/usr/bin/env python3

from __future__ import annotations

from ctypes import CDLL
from ctypes.util import find_library
from functools import lru_cache
from typing import Sequence, Type

import ctypes
import os

_libfile_path = "asciirast_libfile"


def set_libfile_path(path: str):
    global _libfile_path
    _libfile_path = path


def get_libfile_path():
    return _libfile_path


def get_libfile():
    return CDLL(get_libfile_path())


def get_libc():
    return CDLL(find_library("c"))


@lru_cache
def typed_lib_func(
    function_name: str,
    argtypes: Sequence[Type[ctypes._CData]],
    restype: Type[ctypes._CData],
) -> ctypes._NamedFuncPointer:
    f = get_libfile().__getattr__(function_name)
    f.argtypes = argtypes
    f.restype = restype
    return f


@lru_cache
def typed_libc_func(
    function_name: str,
    argtypes: Sequence[Type[ctypes._CData]],
    restype: Type[ctypes._CData],
) -> ctypes._NamedFuncPointer:
    f = get_libfile().__getattr__(function_name)
    f.argtypes = argtypes
    f.restype = restype
    return f
