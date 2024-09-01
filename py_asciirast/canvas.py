from __future__ import annotations

from functools import cached_property, cache
from typing import List, TextIO
from ctypes import c_uint32, c_char, c_void_p
from dataclasses import dataclass

import sys
import ctypes

from py_asciirast._external import typed_lib_func, typed_libc_func
from py_asciirast.color_encoding import RGBColor, RGBColor_c, EncodedRGBColor


@dataclass
class CanvasDepth:
    value: int

    @staticmethod
    def from_float(x: float) -> CanvasDepth:
        return CanvasDepth((int)(x * (2**32 - 1)))

    @staticmethod
    def from_int(x: int) -> CanvasDepth:
        return CanvasDepth(x)


class Canvas:
    def __init__(
        self,
        width: int,
        height: int,
        default_fg_color=RGBColor(255, 255, 255),
        default_bg_color=RGBColor(0, 0, 0),
        default_ascii_char=" ",
    ) -> None:
        self._obj = None

        self._width = width
        self._height = height
        self._default_fg_color_c = default_fg_color.to_rgb_c()
        self._default_bg_color_c = default_bg_color.to_rgb_c()
        self._default_ascii_char_c = ord(default_ascii_char[0])

        f = typed_lib_func(
            "canvas_create",
            (c_uint32, c_uint32, RGBColor_c, RGBColor_c, c_char),
            c_void_p,
        )
        self._obj = f(
            self._width,
            self._height,
            self._default_fg_color_c,
            self._default_bg_color_c,
            self._default_ascii_char_c,
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
            "canvas_get_raw_fg_color_values",
            (c_void_p,),
            ctypes.POINTER(c_uint32),
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(
            ret_ptr, ctypes.POINTER(c_uint32 * self._height * self._width)
        )

    @cached_property
    def _raw_bg_color_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_uint32]]]:
        f = typed_lib_func(
            "canvas_get_raw_bg_color_values",
            (c_void_p,),
            ctypes.POINTER(c_uint32),
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(
            ret_ptr, ctypes.POINTER(c_uint32 * self._height * self._width)
        )

    @cached_property
    def _raw_ascii_char_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_char]]]:
        f = typed_lib_func(
            "canvas_get_raw_ascii_char_values",
            (c_void_p,),
            ctypes.POINTER(c_uint32),
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(ret_ptr, ctypes.POINTER(c_char * self._height * self._width))

    @cached_property
    def _raw_depth_values(
        self,
    ) -> ctypes._Pointer[ctypes.Array[ctypes.Array[ctypes.c_uint32]]]:
        f = typed_lib_func(
            "canvas_get_raw_depth_values",
            (c_void_p,),
            ctypes.POINTER(c_uint32),
        )
        ret_ptr = f(self._obj)
        return ctypes.cast(
            ret_ptr, ctypes.POINTER(c_uint32 * self._height * self._width)
        )

    @property
    def width(self) -> int:
        return self._width

    @property
    def height(self) -> int:
        return self._height

    @property
    def default_fg_color(self) -> RGBColor:
        return self._default_fg_color_c.to_rgb()

    @property
    def default_bg_color(self) -> RGBColor:
        return self._default_bg_color_c.to_rgb()

    @property
    def default_ascii_char(self) -> str:
        return chr(self._default_ascii_char_c)

    def get_fg_color_value_at(self, x: int, y: int) -> RGBColor:
        return self._raw_fg_color_values.contents[x][y]

    def get_all_fg_color_values(self) -> List[List[RGBColor]]:
        return [
            [EncodedRGBColor.decode(val) for val in arr]
            for arr in self._raw_fg_color_values.contents
        ]

    def get_bg_color_value_at(self, x: int, y: int) -> RGBColor:
        return self._raw_bg_color_values.contents[x][y]

    def get_all_bg_color_values(self) -> List[List[RGBColor]]:
        return [
            [EncodedRGBColor.decode(val) for val in arr]
            for arr in self._raw_bg_color_values.contents
        ]

    def get_ascii_char_value_at(self, x: int, y: int) -> str:
        return self._raw_ascii_char_values.contents[x][y]

    def get_all_ascii_char_values(self) -> List[List[str]]:
        return [
            [ascii_char.decode("utf8") for ascii_char in arr]
            for arr in self._raw_ascii_char_values.contents
        ]

    def get_depth_value_at(self, x: int, y: int) -> int:
        return self._raw_depth_values.contents[x][y]

    def get_all_depth_values(self) -> List[List[int]]:
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
        depth_value: CanvasDepth = CanvasDepth.from_int(1),
    ) -> None:
        if not x < self._width or not y < self._height:
            raise ValueError(
                "The point is out of bounds! Use the equivalent draw function if culling is desired."
            )

        fg_color_c = self._default_fg_color_c if not fg_color else fg_color.to_rgb_c()
        bg_color_c = self._default_bg_color_c if not bg_color else bg_color.to_rgb_c()

        f = typed_lib_func(
            "canvas_plot",
            (
                c_void_p,
                c_uint32,
                c_uint32,
                c_uint32,
                RGBColor_c,
                RGBColor_c,
                c_char,
            ),
            None,
        )
        f(
            self._obj,
            x,
            y,
            depth_value.value,
            fg_color_c,
            bg_color_c,
            ord(ascii_char[0]),
        )

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
                ctypes.c_int,
                ctypes.c_char_p,
            ),
            ctypes.c_void_p,
        )
        self.FILE_ptr = fdopen(out.fileno(), ctypes.c_char_p(b"w"))

    def __del__(self) -> None:
        if not self.FILE_ptr:
            return
        fclose = typed_libc_func(
            "fclose",
            (ctypes.c_void_p,),
            None,
        )
        fclose(self.FILE_ptr)
