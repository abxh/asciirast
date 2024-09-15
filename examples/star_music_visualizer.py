from time import sleep
from typing import Tuple

import math
from threading import Thread

from py_asciirast import Canvas, CanvasDepth, RGBColor, draw_line
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


def render_star(
    canvas: Canvas, zoom: float, angle: float, fg_color: RGBColor, depth: int
) -> None:
    for i in range(5):
        vec_base = (zoom, -zoom)

        angle_rad1 = math.radians(angle + (72 + 72) * i)
        angle_rad2 = math.radians(angle + (72 + 72) * (i + 1))

        vec1 = rotate_2d(vec_base, angle_rad1)
        vec2 = rotate_2d(vec_base, angle_rad2)

        vp1 = get_pixel_coords(vec1, canvas.width, canvas.height)
        vp2 = get_pixel_coords(vec2, canvas.width, canvas.height)

        draw_line(canvas, *vp1, *vp2, "*", fg_color, depth=CanvasDepth.from_int(depth))


def analyze_audio(filename: str) -> Tuple[float, list[float], list[float]]:
    # see:
    # https://stackoverflow.com/a/51436401
    # https://stackoverflow.com/a/43924448
    # https://stackoverflow.com/q/51305317
    # https://librosa.org/doc/main/auto_examples/plot_display.html
    # https://librosa.org/doc/0.8.1/generated/librosa.beat.tempo.html
    # https://librosa.org/doc/main/generated/librosa.piptrack.html
    # https://gitlab.com/avirzayev/music-visualizer

    import librosa
    import numpy as np
    import scipy

    import matplotlib.pyplot as plt

    # following is taken from documentation:
    y, sr = librosa.load(filename, mono=True)
    S = np.abs(librosa.stft(y, hop_length=512, n_fft=2048 * 4))

    pitches, magnitudes = librosa.piptrack(
        S=S, sr=sr, threshold=1, ref=np.mean, hop_length=512, n_fft=2048 * 4
    )

    max_indexes = np.argmax(magnitudes, axis=0)
    pitch = pitches[max_indexes, range(magnitudes.shape[1])]

    onset_env = librosa.onset.onset_strength(y=y, sr=sr)
    prior_lognorm = scipy.stats.lognorm(loc=np.log(120), scale=120, s=1)
    tempo = librosa.feature.tempo(
        onset_envelope=onset_env, sr=sr, aggregate=None, prior=prior_lognorm
    )

    # make tempo = [0; 1], pitch = [0; 1]
    tempo = (tempo - np.min(tempo)) / np.ptp(tempo)
    pitch = (pitch - np.min(pitch)) / np.ptp(pitch)

    # fig, axs = plt.subplots(2)
    # fig.suptitle("tempo (first), pitch (second)")
    # axs[0].plot(tempo)
    # axs[1].plot(pitch)
    # plt.show()

    frametime = librosa.core.get_duration(
        y=y, hop_length=512, n_fft=2048 * 4
    ) / np.size(tempo)

    return frametime, tempo.tolist(), pitch.tolist()


def play_audio(filename: str):
    # see:
    # https://stackoverflow.com/a/64060809

    import librosa
    import sounddevice as sd
    import time

    y, sr = librosa.load(filename, mono=True)

    duration = librosa.core.get_duration(y=y)

    sd.play(y, sr)

    time.sleep(duration)


def main() -> None:
    canvas = asciirast.Canvas(50, 25)

    import librosa

    filename = librosa.example("trumpet")

    # print("music file name (relative path): ", end = "")
    # filename = input()

    frametime, tempo, pitch = analyze_audio(filename)

    def loop():
        angle = 0
        with asciirast.hidden_cursor():
            for i in range(len(tempo)):
                render_star(
                    canvas,
                    pitch[i] * 0.5 + 0.5,
                    angle,
                    RGBColor(240, 240, 240),
                    depth=3,
                )

                angle -= 5 * tempo[i]

                canvas.print_formatted()
                canvas.clear()
                asciirast.move_up_lines(canvas.height)

                sleep(frametime * 0.5)

    t1 = Thread(target=play_audio, args=(filename,))
    t2 = Thread(target=loop)
    t1.start()
    t2.start()
    t1.join()
    t2.join()


if __name__ == "__main__":
    main()
