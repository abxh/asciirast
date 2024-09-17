from ctypes import c_float, c_void_p, c_uint32, c_char

from py_asciirast.canvas import Canvas, CanvasDepth
from py_asciirast.color_encoding import RGBColor, RGBColor_c
from py_asciirast.external import typed_lib_func


def draw_point(
    canvas: Canvas,
    x: float,
    y: float,
    ascii_char: str,
    fg_color: RGBColor | None = None,
    bg_color: RGBColor | None = None,
    depth: CanvasDepth = CanvasDepth.from_int(1),
) -> None:
    fg_color_c = canvas._default_fg_color_c if not fg_color else fg_color.to_rgb_c()
    bg_color_c = canvas._default_bg_color_c if not bg_color else bg_color.to_rgb_c()

    f = typed_lib_func(
        "draw_point",
        (c_void_p, c_float, c_float, c_uint32, RGBColor_c, RGBColor_c, c_char),
        None,
    )
    f(canvas._obj, x, y, depth.value, fg_color_c, bg_color_c, ord(ascii_char[0]))


def draw_line(
    canvas: Canvas,
    x0: float,
    y0: float,
    x1: float,
    y1: float,
    ascii_char: str,
    fg_color: RGBColor | None = None,
    bg_color: RGBColor | None = None,
    depth: CanvasDepth = CanvasDepth.from_int(1),
) -> None:
    fg_color_c = canvas._default_fg_color_c if not fg_color else fg_color.to_rgb_c()
    bg_color_c = canvas._default_bg_color_c if not bg_color else bg_color.to_rgb_c()

    f = typed_lib_func(
        "draw_line",
        (
            c_void_p,
            c_float,
            c_float,
            c_float,
            c_float,
            c_uint32,
            RGBColor_c,
            RGBColor_c,
            c_char,
        ),
        None,
    )
    f(
        canvas._obj,
        x0,
        y0,
        x1,
        y1,
        depth.value,
        fg_color_c,
        bg_color_c,
        ord(ascii_char[0]),
    )
