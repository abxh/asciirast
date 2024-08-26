from dataclasses import dataclass
from ctypes import c_uint8, c_uint32, Structure

from rasterizer_api._external import typed_lib_func


@dataclass
class RGBColor:
    r: int
    g: int
    b: int


class RGBColor_c(Structure):
    _fields_ = [("r", c_uint8), ("g", c_uint8), ("b", c_uint8)]


def encode_rgb(rgb: RGBColor) -> int:
    f = typed_lib_func("color_encode_rgb", (c_uint8, c_uint8, c_uint8), c_uint32)
    return f(rgb.r, rgb.g, rgb.b)


def decode_rgb(rgb: int) -> RGBColor:
    f = typed_lib_func("color_decode_rgb", (c_uint32,), RGBColor_c)
    res = f(rgb)
    return RGBColor(res.r, res.g, res.b)
