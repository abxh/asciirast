from time import sleep
from math import cos, sin

import math

from py_asciirast import Canvas, CanvasDepth

import py_asciirast as asciirast


def frame_renderer(canvas: Canvas):
    # see following for detailed explanation and code with comments:
    # https://www.a1k0n.net/2011/07/20/donut-math.html

    theta_spacing = 0.07
    phi_spacing = 0.02

    R1 = 1
    R2 = 2
    K2 = 5
    K1 = canvas.width * (2 / 10) * K2 * 1 / (R1 + R2)

    def frame_render(A: float, B: float) -> None:
        cosA, sinA = cos(A), sin(A)
        cosB, sinB = cos(B), sin(B)

        theta = -theta_spacing
        while theta < 2 * math.pi:
            theta += theta_spacing
            costheta, sintheta = cos(theta), sin(theta)

            phi = -phi_spacing
            while phi < 2 * math.pi:
                phi += phi_spacing
                cosphi, sinphi = cos(phi), sin(phi)

                circlex = R2 + R1 * costheta
                circley = R1 * sintheta

                # taken from math deduced in the article:
                x = (
                    circlex * (cosB * cosphi + sinA * sinB * sinphi)
                    - circley * cosA * sinB
                )
                y = (
                    circlex * (sinB * cosphi - sinA * cosB * sinphi)
                    + circley * cosA * cosB
                )
                z = K2 + cosA * circlex * sinphi + circley * sinA
                one_over_z = 1 / z

                xp = int(canvas.width / 2 + K1 * one_over_z * x)
                yp = int(canvas.height / 2 - K1 * one_over_z * y)

                L = (
                    cosphi * costheta * sinB
                    - cosA * costheta * sinphi
                    - sinA * sintheta
                    + cosB * (cosA * sintheta - costheta * sinA * sinphi)
                )

                if L > 0:
                    luminance_index = int(L * 8)
                    canvas.plot(
                        xp,
                        yp,
                        ".,-~:;=!*#$@"[luminance_index],
                        depth_value=CanvasDepth.from_float(one_over_z),
                    )

    return frame_render


def main() -> None:
    canvas = asciirast.Canvas(50, 25)
    render = frame_renderer(canvas)

    A = 0
    B = 0

    with asciirast.hidden_cursor():
        while True:
            render(A, B)

            A += 0.07
            B += 0.07
            sleep(0.1)

            canvas.print_formatted()
            canvas.clear()
            asciirast.move_up_lines(canvas.height)

    canvas.clear()
    render(A, B)
    canvas.print_formatted()


if __name__ == "__main__":
    main()
