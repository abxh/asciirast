from ctypes import c_uint16, c_void_p, c_uint32, c_char

from py_asciirast.canvas import Canvas
from py_asciirast.color_encoding import RGBColor, RGBColor_c, to_rgb_c
from py_asciirast._external import typed_lib_func


def draw_point(
    canvas: Canvas,
    x: int,
    y: int,
    ascii_char: str,
    fg_color: RGBColor | None = None,
    bg_color: RGBColor | None = None,
    z_order: int = 1,
) -> None:
    if not len(ascii_char) == 1:
        raise ValueError("must be a single char")
    if not 32 <= ord(ascii_char) <= 126:
        raise ValueError("ascii character not representable when printed")

    fg_color_c = to_rgb_c(canvas.default_fg_color if not fg_color else fg_color)
    bg_color_c = to_rgb_c(canvas.default_bg_color if not bg_color else bg_color)

    f = typed_lib_func(
        "draw_point",
        (c_void_p, c_uint32, c_uint32, RGBColor_c, RGBColor_c, c_char, c_uint16),
        None,
    )
    f(canvas._obj, x, y, fg_color_c, bg_color_c, ord(ascii_char), z_order)
