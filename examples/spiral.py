from time import sleep
from typing import Tuple

import math

from py_asciirast import Canvas, RGBColor

import py_asciirast as asciirast


def rotate_2d(xy: Tuple[float, float], radians: float) -> Tuple[float, float]:
    x, y = xy
    return (
        x * math.cos(radians) - y * math.sin(radians),
        x * math.sin(radians) + y * math.cos(radians),
    )


def get_pixel_coords(
    xy: Tuple[float, float], width: int, height: int
) -> Tuple[float, float]:
    x, y = xy
    px, py = (x + 1.0) / 2.0 * width, (y + 1.0) / 2.0 * height
    return px, height - 1 - py 


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

    def render():
        nonlocal curr_angle_deg
        for o in range(6):
            for i in range(7):
                angle_rad = math.radians(curr_angle_deg - 20 * i - 60 * o)
                vec = rotate_2d((0.6 - 0.1 * i, 0.6 - 0.1 * i), angle_rad)
                color = interpolate_color(color_red, color_yellow, i / 6)
                asciirast.draw_point(
                    canvas,
                    *get_pixel_coords(vec, canvas.width, canvas.height),
                    ascii_palette[i],
                    color
                )
        curr_angle_deg -= 10

    with asciirast.hidden_cursor():
        while True:
            render()

            canvas.print_formatted()
            canvas.clear()
            asciirast.move_up_lines(canvas.height)

            sleep(1 / 10)

    canvas.clear()
    render()
    print("\n", end="")
    canvas.print_formatted()


if __name__ == "__main__":
    main()
