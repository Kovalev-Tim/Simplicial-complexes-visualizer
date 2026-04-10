# Simplicial Complex Visualizer

A C++ OpenGL tool for constructing, and interactively visualizing simplicial complexes (up to dimension 3) using force-based and annealing methods.

# Features

- Supports vertices, edges, triangles, and tetrahedra
- Automatic closure of simplicial complexes
- Real-time embedding in 3D
- Hybrid optimization:
  - force-based dynamics
  - stochastic annealing
- Camera controls (orbit / pan / zoom)

---

# Project structure

- `src/` - source code
- `src/core/` - complex and embedding classes
- `src/render/` - OpenGL renderer
- `src/main.cpp` - main function
- `build/` - build directory
- `glfw/` - GLFW header files
- `glad/` - glad header files

# Getting Started

## 1. Clone

```
git clone https://github.com/Kovalev-Tim/Simplex-visualizer.git
cd simplicial-visualizer
```

## 2. Build with CMake

```
mkdir build
cd build
cmake ..
cmake --build .
```

## 3. Run
in build directory
`.\Debug\viewer.exe`

## 4. Input format
- 1st line: number of simplicies in a complex
- for each simplex line: dimension of the simplex, followed by space-separated vertices

Example:
```
3
3 0 1 2
2 2 3
3 3 4 5
```

## 5. Controls

- left mouse button: orbit camera
- right mouse button: pan camera
- middle mouse button: zoom camera
- space: simulate annealing steps

## 6. Tuning parameters

- `k` in embedding-points.hpp - force constant
- `anneal_temp` in main.cpp - starting temperature for annealing
- `freopen("../input.txt", "r", stdin);` in main.cpp - read input from file (torus complex)
- run simplex and embedding tests in main.cpp
- `annealing_steps_per_space` in main.cpp - number of annealing steps per space key press