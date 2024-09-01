from typing import TextIO
from contextlib import contextmanager

import sys


def move_up_lines(n: int, out: TextIO = sys.stdout) -> None:
    out.write(f"\033[{n}A\r")


@contextmanager
def hidden_cursor(out: TextIO = sys.stdout):
    try:
        out.write("\033[?25l")
        yield None
        out.write("\033[?25h")
    except KeyboardInterrupt:
        out.write("\033[?25h")
