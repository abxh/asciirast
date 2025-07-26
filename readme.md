# asciirast

A software rasterizer based on the classical OpenGL graphics pipeline made for educational purposes.
It is meant to both render ascii art and real images. It currently simulates vertex and fragment shaders.

## Dependencies

For the examples:
- `libsdl2-dev`
- `libsdl2-ttf-dev`

Set `ASCIIRAST_SDL_EXAMPLES` to false if desired.

## Build steps

Build in release mode preferably for preview:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build_release
cmake --build build_release -j $(nproc)
```
Then the executables will be visible in `build_release/bin` directory. Use `-j %NUMBER_OF_PROCESSORS%` on windows.

Alternatively, build in debug mode and export compile commands for LSP support:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build
cmake --build build -j $(nproc) 
```

## Resources
### Start here:
- How Do Computers Display 3D on a 2D Screen?: https://www.youtube.com/watch?v=eoXn6nwV694
- Introduction to Computer Graphics: https://www.youtube.com/watch?v=vLSphLtKQ0o&list=PLplnkTzzqsZTfYh4UbhLGpI5kGd5oW_Hh
- Perspective Projection Matrix: https://www.youtube.com/watch?v=EqNcqBdrNyI

### Inspiration:
- https://github.com/nikolausrauch/software-rasterizer
- https://github.com/ssloy/tinyrenderer/wiki
- https://github.com/keith2018/SoftGLRender
