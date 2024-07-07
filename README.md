# ascii-rasterzier

This is a fixed-pipeline software rasterizer that's plots ascii characters to the console / terminal.

Dependencies:
- `TrueColor ANSI` supporting terminal.
- `SDL2`
- `SDL2_ttf`

Supports:
- Drawing points, lines and triangles with interpolated colors and ascii characters -- given an ascii palette.
- Trinangle backface culling.
- Screen space clipping.

Algorithms used:
- Linear interpolation for line plotting.
- Baycentric coordinates for triangle plotting.
