# asciirast

## Build steps
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
Now the executables are visible in `build/bin` directory.

You may need to install the `libsan` and `ubsan` sanitizers if not already. Otherwise disable them in the cmake build script.

## Inspiration
- https://github.com/nikolausrauch/software-rasterizer
