from __future__ import annotations

from functools import cached_property, cache
from ctypes import c_int, c_char_p, c_uint32, c_float, c_char, c_void_p
from typing import List, TextIO

from py_asciirast._external import typed_lib_func, typed_libc_func
from py_asciirast.color_encoding import RGBColor, RGBColor_c, decode_rgb, to_rgb_c

import sys
import ctypes


class Canvas:
    def __init__(
        self,
        width: int,
        height: int,
        default_fg_color: RGBColor = RGBColor(255, 255, 255),
        default_bg_color: RGBColor = RGBColor(0, 0, 0),
        default_ascii_char: str = " ",
    ) -> None:
        self._obj = None

        if not len(default_ascii_char) == 1:
            raise ValueError("must be a single char")
        if not 32 <= ord(default_ascii_char) <= 126:
            raise ValueError("ascii character not representable when printed")

        self.width = width
        self.height = height
        self.default_fg_color = default_fg_color
        self.default_bg_color = default_bg_color
        self.default_ascii_char = default_ascii_char

        f = typed_lib_func(
            "canvas_create",
            (c_uint32, c_uint32, RGBColor_c, RGBColor_c, c_char),
            c_void_p,
        )
        self._obj = f(
            self.width,
            self.height,
            to_rgb_c(self.default_fg_color),
            to_rgb_c(self.default_bg_color),
            ord(self.default_ascii_char),
        )

    def __del__(self) -> None:
        if not self._obj:
            return
        f = typed_lib_func("canvas_destroy", (c_void_p,), None)
        f(self._obj)

    @cached_property
    def _raw_fg_color_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_uint32]]]:
        f = typed_lib_func(
            "canvas_get_raw_fg_color_values", (c_void_p,), ctypes.POINTER(c_uint32)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_uint32 * self.height * self.width))

    @cached_property
    def _raw_bg_color_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_uint32]]]:
        f = typed_lib_func(
            "canvas_get_raw_bg_color_values", (c_void_p,), ctypes.POINTER(c_uint32)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_uint32 * self.height * self.width))

    @cached_property
    def _raw_ascii_char_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_char]]]:
        f = typed_lib_func(
            "canvas_get_raw_ascii_char_values", (c_void_p,), ctypes.POINTER(c_uint32)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_char * self.height * self.width))

    @cached_property
    def _raw_depth_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_float]]]:
        f = typed_lib_func(
            "canvas_get_raw_depth_values", (c_void_p,), ctypes.POINTER(c_float)
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(ctypes.c_float * self.height * self.width))

    @property
    def fg_color_values(self) -> List[List[RGBColor]]:
        return [
            [decode_rgb(rgb) for rgb in arr]
            for arr in self._raw_fg_color_values.contents
        ]

    @property
    def bg_color_values(self) -> List[List[RGBColor]]:
        return [
            [decode_rgb(rgb) for rgb in arr]
            for arr in self._raw_bg_color_values.contents
        ]

    @property
    def ascii_char_values(self) -> List[List[str]]:
        return [
            [ascii_char.decode("utf8") for ascii_char in arr]
            for arr in self._raw_ascii_char_values.contents
        ]

    @property
    def depth_values(self) -> List[List[str]]:
        return [[d for d in arr] for arr in self._raw_depth_values.contents]

    def clear(self) -> None:
        f = typed_lib_func("canvas_clear", (c_void_p,), None)
        f(self._obj)

    def plot(
        self,
        x: int,
        y: int,
        ascii_char: str,
        fg_color: RGBColor | None = None,
        bg_color: RGBColor | None = None,
        depth_value: float = 0.01,
    ) -> None:
        if not 0 <= x < self.width or not 0 <= y < self.height:
            return
        if not len(ascii_char) == 1:
            raise ValueError("must be a single char")
        if not 32 <= ord(ascii_char) <= 126:
            raise ValueError("ascii character not representable when printed")
        if not 0.0 <= depth_value <= 1.0:
            raise ValueError("depth value must be between 0.0 and 1.0")

        fg_color_c = to_rgb_c(self.default_fg_color if not fg_color else fg_color)
        bg_color_c = to_rgb_c(self.default_bg_color if not bg_color else bg_color)

        f = typed_lib_func(
            "canvas_plot",
            (c_void_p, c_uint32, c_uint32, c_float, RGBColor_c, RGBColor_c, c_char),
            None,
        )
        f(self._obj, x, y, depth_value, fg_color_c, bg_color_c, ord(ascii_char))

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
