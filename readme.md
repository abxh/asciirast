# asciirast

A software rasterizer based on the classical OpenGL graphics pipeline made for educational purposes.
It is meant to both render ascii art and real images. It currently simulates vertex and fragment shaders.

## Dependencies
For SDL-based examples:
- `libsdl2-dev`
- `libsdl2-ttf-dev`

## Build steps
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -G "Ninja" ..
cmake --build .
```
Now the executables are visible in `build/bin` directory.

## Start here
- How Do Computers Display 3D on a 2D Screen?: https://www.youtube.com/watch?v=eoXn6nwV694
- Introduction to Computer Graphics: https://www.youtube.com/watch?v=vLSphLtKQ0o&list=PLplnkTzzqsZTfYh4UbhLGpI5kGd5oW_Hh
- Perspective Projection Matrix: https://www.youtube.com/watch?v=EqNcqBdrNyI

## Inspiration
- https://github.com/nikolausrauch/software-rasterizer
- https://github.com/ssloy/tinyrenderer/wiki
- https://github.com/keith2018/SoftGLRender
