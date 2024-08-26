#!/usr/bin/env python3

from rasterizer_api.canvas import Canvas
from rasterizer_api.color import RGBColor

# from colorama import just_fix_windows_console
#
# just_fix_windows_console()


c = Canvas(50, 25)
chars = c.raw_char_values.contents

c.plot(49, 24, "a")
print(chars[49][24])

c.clear()
print(chars[49][24])

c.plot(49, 24, "b")
print(chars[49][24])

c.plot(49, 24, "c")
print(chars[49][24])

for i in range(50):
    c.plot(i * 2, i, "*", bg_color=RGBColor(90, 90, 90))

c.print_formatted(with_bg=True)
