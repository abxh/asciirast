# asciirast

A software rasterizer based on the classical OpenGL graphics pipeline made for educational purposes.
It is meant to both render ascii art and real images. It currently simulates vertex and fragment shaders.

## Dependencies
For SDL-based examples:
- `libsdl2-dev`

## Build steps
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```
Now the executables are visible in `build/bin` directory.

## Start here
- https://www.youtube.com/watch?v=eoXn6nwV694
- https://www.youtube.com/watch?v=vLSphLtKQ0o&list=PLplnkTzzqsZTfYh4UbhLGpI5kGd5oW_Hh

## Inspiration
- https://github.com/nikolausrauch/software-rasterizer
- https://github.com/ssloy/tinyrenderer/wiki
