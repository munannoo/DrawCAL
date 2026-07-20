# Project Visualizer

## Build Instructions
To build the project package, run:
```bash
mkdir build
cd build
cmake ..
```
## Compile in VS Code
Press:
```text
Ctrl + Shift + B
```
## Compile in cmd
```text
cmake -S . -B build (Generates Build Files to build)
cmake --build build (Builds the project)
```

## Run

Executable location:
```text
drawcal/drawcal.exe
```

## R3D Implementation
R3D is a useful renderer as it contains a bunch of funtions that are useful for a 3d workspace.
R3D contains native light objects and enough support so that external shaders aren't necessary, it also contains generate normals and tangents function which will be useful for vertex editing.
R3D however uses a different method of compilation from raylib, R3D_Begin(Camera) ... R3D_End() is used to start and end the rendering of a 3d scene, this is different from raylib's BeginMode3D(Camera) ... EndMode3D() which is used to start and end the rendering of a 3d scene.
Both of these are used, however, for rendering different materials