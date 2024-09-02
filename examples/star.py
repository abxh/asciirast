from time import sleep
from typing import Tuple

import math

from py_asciirast import Canvas, RGBColor

import py_asciirast as asciirast
from py_asciirast.draw import draw_line


def rotate_2d(xy: Tuple[float, float], radians: float) -> Tuple[float, float]:
    x, y = xy
    return (
        x * math.cos(radians) - y * math.sin(radians),
        x * math.sin(radians) + y * math.cos(radians),
    )


def get_pixel_coords(xy: Tuple[float, float], canvas: Canvas) -> Tuple[int, int]:
    x, y = xy
    px, py = (x + 1.0) / 2.0 * canvas._width, (y + 1.0) / 2.0 * canvas._height
    return int(px), canvas._height - 1 - int(py)


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

    def render():
        draw_line(canvas, -1, 0, 1, 24, "*")

    with asciirast.hidden_cursor():
        while True:
            render()

            canvas.print_formatted()
            canvas.clear()
            asciirast.move_up_lines(canvas._height)

            sleep(1 / 10)

    canvas.clear()
    render()
    canvas.print_formatted()


if __name__ == "__main__":
    main()
