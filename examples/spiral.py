import math
from time import sleep
from typing import Tuple

import py_asciirast as asciirast
from py_asciirast.color_encoding import RGBColor
import py_asciirast.printutils as printutils
from py_asciirast.draw import draw_point
from py_asciirast import Canvas


def rotate_2d(xy: Tuple[float, float], radians: float) -> Tuple[float, float]:
    x, y = xy
    return (
        x * math.cos(radians) - y * math.sin(radians),
        x * math.sin(radians) + y * math.cos(radians),
    )


def get_pixel_coords(xy: Tuple[float, float], canvas: Canvas) -> Tuple[int, int]:
    x, y = xy
    px, py = (x + 1.0) / 2.0 * canvas.w, (y + 1.0) / 2.0 * canvas.h
    return int(px), canvas.h - 1 - int(py)


def interpolate_float(x: float, y: float, t: float) -> float:
    return (1 - t) * x + t * y


def interpolate_color(x: RGBColor, y: RGBColor, t: float) -> RGBColor:
    return RGBColor(
        r=int(interpolate_float(x.r, y.r, t)),
        g=int(interpolate_float(x.g, y.g, t)),
        b=int(interpolate_float(x.b, y.b, t)),
    )


def main() -> None:
    canvas = asciirast.Canvas(50, 25)
    ascii_palette = "@%#*+=-:. "
    color_red = RGBColor(255, 0, 0)
    color_yellow = RGBColor(255, 255, 0)

    curr_angle_deg = 0
    with printutils.hidden_cursor():
        while True:
            for o in range(6):
                for i in range(7):
                    angle_rad = math.radians(curr_angle_deg - 20 * i - 60 * o)
                    vec = rotate_2d((0.6 - 0.1 * i, 0.6 - 0.1 * i), angle_rad)
                    color = interpolate_color(color_red, color_yellow, i / 6)
                    draw_point(canvas, *get_pixel_coords(vec, canvas), ascii_palette[i], color)
            curr_angle_deg -= 10
            sleep(1 / 10)

            canvas.print_formatted()
            canvas.clear()
            printutils.move_up_lines(canvas.h)


if __name__ == "__main__":
    main()
