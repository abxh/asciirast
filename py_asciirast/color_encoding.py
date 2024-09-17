from __future__ import annotations

from dataclasses import dataclass
from ctypes import c_uint8, c_uint32, Structure

from py_asciirast.external import typed_lib_func


@dataclass
class RGBColor:
    r: int
    g: int
    b: int

    def to_rgb_c(self) -> RGBColor_c:
        return RGBColor_c(self.r, self.g, self.b)

    def encode(self) -> EncodedRGBColor:
        f = typed_lib_func("color_encode_rgb", (c_uint8, c_uint8, c_uint8), c_uint32)
        return EncodedRGBColor(f(self.r, self.g, self.b))


@dataclass
class EncodedRGBColor:
    value: int

    def decode(self) -> RGBColor:
        f = typed_lib_func("color_decode_rgb", (c_uint32,), RGBColor_c)
        res = f(self.value)
        return RGBColor(res.r, res.g, res.b)


class RGBColor_c(Structure):
    _fields_ = [("r", c_uint8), ("g", c_uint8), ("b", c_uint8)]

    def to_rgb(self) -> RGBColor:
        return RGBColor(self.r, self.g, self.b)

    def encode(self) -> EncodedRGBColor:
        f = typed_lib_func("color_encode_rgb", (c_uint8, c_uint8, c_uint8), c_uint32)
        return EncodedRGBColor(f(self.r, self.g, self.b))
