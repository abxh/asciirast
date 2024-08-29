from typing import TextIO
import sys

from contextlib import contextmanager


def move_up_lines(n: int, out: TextIO = sys.stdout) -> None:
    out.write(f"\033[{n}A\r")


@contextmanager
def hidden_cursor(out: TextIO = sys.stdout):
    try:
        out.write("\033[?25l")
        yield None
        out.write("\033[?25h")
    except:
        out.write("\033[?25h")
