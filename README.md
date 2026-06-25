# Project Visualizer

## Build Instructions

To build the project package, run:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Compile in VS Code

Press:

```text
Ctrl + Shift + B
```

## Run

Executable location:

```text
drawcal/drawcal.exe
```

## Credits / Third-Party References

### rPBR

This project’s physically based rendering lighting and shader implementation was adapted with reference to **rPBR – Physically Based Rendering viewer for raylib**, created by **Victor Fisac**.

rPBR was used as a reference for the PBR shader structure, including the metalness/roughness workflow, Cook-Torrance style lighting, normal mapping, and point-light based material shading.

The implementation in this project has been modified and integrated into DrawCAL’s own rendering, object, material, and lighting systems. The full rPBR viewer/core was not copied directly; only the PBR lighting and shader approach was adapted.

rPBR is licensed under the **zlib/libpng license**.

Original rPBR copyright:

```text
Copyright (c) 2017-2020 Victor Fisac
```

This attribution is included to clearly acknowledge the original source and to avoid misrepresenting the adapted code as fully original.
