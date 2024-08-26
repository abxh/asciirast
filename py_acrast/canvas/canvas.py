from __future__ import annotations

from functools import cached_property, cache
from ctypes import c_int, c_char_p, c_uint32, c_float, c_char, c_void_p
from typing import TextIO

from py_acrast.color.rgbcolor import RGBColor, encode_rgb
from py_acrast._external import typed_lib_func, typed_libc_func

import sys
import ctypes


class Canvas:
    def __init__(
        self,
        w: int,
        h: int,
        default_fg: RGBColor = RGBColor(255, 255, 255),
        default_bg: RGBColor = RGBColor(0, 0, 0),
        default_depth: int = 0,
        default_ascii_char: str = " ",
    ) -> None:
        self._obj = None

        if not 0 <= default_depth <= 1:
            raise ValueError("depth must be between 0 and 1")
        if not len(default_ascii_char) == 1:
            raise ValueError("must be a single char")
        if not 32 <= ord(default_ascii_char) <= 126:
            raise ValueError("ascii character not representable when printed")

        self.w = w
        self.h = h
        self._default_fg_encoded = encode_rgb(default_fg)
        self._default_bg_encoded = encode_rgb(default_bg)
        self._default_depth = default_depth

        f = typed_lib_func(
            "canvas_create",
            (c_uint32, c_uint32, c_uint32, c_uint32, c_float, c_char),
            c_void_p,
        )
        self._obj = f(
            self.w,
            self.h,
            self._default_fg_encoded,
            self._default_bg_encoded,
            self._default_depth,
            ord(default_ascii_char),
        )

    def __del__(self) -> None:
        if not self._obj:
            return
        f = typed_lib_func("canvas_destroy", (c_void_p,), None)
        f(self._obj)

    @cached_property
    def raw_fg_color_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_uint32]]]:
        f = typed_lib_func(
            "canvas_get_raw_fg_color_values", (c_void_p,), ctypes.POINTER(c_uint32)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_uint32 * self.h * self.w))

    @cached_property
    def raw_bg_color_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_uint32]]]:
        f = typed_lib_func(
            "canvas_get_raw_bg_color_values", (c_void_p,), ctypes.POINTER(c_uint32)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_uint32 * self.h * self.w))

    @cached_property
    def raw_char_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_char]]]:
        f = typed_lib_func(
            "canvas_get_raw_char_values", (c_void_p,), ctypes.POINTER(c_uint32)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_char * self.h * self.w))

    @cached_property
    def raw_depth_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_float]]]:
        f = typed_lib_func(
            "canvas_get_raw_depth_values", (c_void_p,), ctypes.POINTER(c_float)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_float * self.h * self.w))

    def clear(self) -> None:
        f = typed_lib_func("canvas_clear", (c_void_p,), None)
        f(self._obj)

    def plot(
        self,
        x: int,
        y: int,
        c: str,
        fg_color: RGBColor | None = None,
        bg_color: RGBColor | None = None,
        depth: float | None = None,
    ) -> None:
        if not 0 <= x < self.w or not 0 <= y < self.h:
            return
        if not len(c) == 1:
            raise ValueError("must be a single char")
        if not 32 <= ord(c) <= 126:
            raise ValueError("ascii character not representable when printed")

        fg_color_encoded = (
            self._default_fg_encoded if not fg_color else encode_rgb(fg_color)
        )
        bg_color_encoded = (
            self._default_bg_encoded if not bg_color else encode_rgb(bg_color)
        )
        depth_value = self._default_depth if not depth else depth

        f = typed_lib_func(
            "canvas_plot",
            (c_void_p, c_uint32, c_uint32, c_float, c_uint32, c_uint32, c_char),
            None,
        )
        f(self._obj, x, y, depth_value, fg_color_encoded, bg_color_encoded, ord(c))

    def print_formatted(self, out: TextIO = sys.stdout, with_bg: bool = False) -> None:
        f = typed_lib_func(
            "canvas_print_formatted" + ("_wo_bg" if not with_bg else ""),
            (c_void_p, c_void_p),
            None,
        )
        f(self._obj, OutputFileDescriptor_c(out).FILE_ptr)


@cache
class OutputFileDescriptor_c:
    def __init__(self, out: TextIO) -> None:
        self.FILE_ptr = None

        fdopen = typed_libc_func(
            "fdopen",
            (
                c_int,
                c_char_p,
            ),
            c_void_p,
        )
        self.FILE_ptr = fdopen(out.fileno(), c_char_p(b"w"))

    def __del__(self) -> None:
        if not self.FILE_ptr:
            return
        fclose = typed_libc_func(
            "fclose",
            (c_void_p,),
            None,
        )
        fclose(self.FILE_ptr)
