# FOpenGL

**FOpenGL** is a C++ educational and research project focused on **OpenGL** and real-world rendering techniques.  
It is designed as a sandbox for learning graphics programming, experimenting with rendering algorithms, and practicing clean C++/CMake project structure.

The project intentionally prioritizes **clarity, explicit architecture, and correctness** over abstraction-heavy or engine-like solutions.

---

## Project Goals

- Practical understanding of **OpenGL (3.3+)**
- Implementation of a classic **forward rendering pipeline**
- Deep dive into all ascpects of rendering
- Clean CMake setup with real dependency management
- Tooling for profiling, debugging, and static analysis
- Solid foundation for future rendering experiments

---

## Features

### Rendering
- Forward rendering pipeline
- Directional light with **shadow mapping**
- **PCF (Percentage Closer Filtering)** for soft shadows
- Point lights and spot lights
- Material system (specular / shininess)
- 2D textures

### Camera & Input
- FPS-style camera (WASD + mouse)
- Raw mouse input (when supported)
- Game / UI input mode switching

### Models
- Model loading via **Assimp**
- Per-model transforms:
  - Translation
  - Rotation (degrees)
  - Scale
- Runtime manipulation via ImGui

### UI (optional)
- Dear ImGui integration
- Editing:
  - Directional / point lights
  - Light parameters
  - Model transforms

---

## Directional Shadow Mapping

The project implements a classic **two-pass shadow mapping** approach:

### 1. Shadow Pass
- Scene rendered from the light’s point of view
- Depth written into a dedicated framebuffer
- Orthographic projection for directional light

### 2. Main Pass
- Shadow comparison against the depth map
- Depth bias to fight **shadow acne**
- **PCF (3×3)** sampling for smoother shadow edges
- Frustum clipping to avoid invalid shadow lookups

Implemented techniques:
- Normal-dependent depth bias
- Out-of-frustum shadow rejection
- Configurable shadow map resolution

---

## Architecture Overview

The codebase is structured around explicit rendering concepts:


