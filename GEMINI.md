# Godot Procedural Dungeon Generator - Context & Guide

## Project Overview

This is a **Godot 4.x GDExtension** project written in C++ that provides high-performance procedural generation algorithms for dungeons and levels. It includes implementations for:
- **BSP (Binary Space Partitioning):** Classic roguelike rooms and corridors.
- **Walker:** Organic, cave-like structures using random walks.
- **Tiling WFC (Wave Function Collapse):** Constraint-based tile placement.
- **Overlapping WFC:** Pattern-based generation from example images.

The project is designed to be built as a C++ library that integrates natively into Godot, offering significant performance advantages over pure GDScript implementations, especially for complex algorithms like WFC.

## Architecture & Technologies

- **Language:** C++17 (Core logic), GDScript (Editor tools/Plugin).
- **Build System:** CMake (3.9+).
- **Framework:** Godot GDExtension (via `godot-cpp`).
- **External Libraries:**
    - `fast-wfc`: Efficient WFC implementation.
    - `godot-cpp`: C++ bindings for the Godot GDExtension API.

## Directory Structure

*   `src/`: **Core C++ Source Code.** Contains the implementations of the generators (`bsp_godot`, `walker`, `gdwfc_v2`) and the GDExtension registration logic (`register_types`).
*   `addons/dungeon_generator/`: **Godot Addon Source.** Contains the editor plugin scripts (`plugin.gd`), the visual preview node (`DungeonPreview.gd`), and the `.gdextension` configuration file.
*   `godot-cpp/`: Submodule for Godot C++ bindings.
*   `tiling-wfc/` & `fast-wfc/`: Submodules for WFC algorithms.
*   `build/`: Build artifacts (created by CMake).
    *   `deploy/addons/wfc/`: The final "ready-to-deploy" addon folder generated after a successful build.
*   `docs/`: Documentation for specific algorithms and build instructions.
*   `CMakeLists.txt`: Main build configuration.

## Build & Installation

The project uses CMake to build the shared library (`.dll`, `.so`, or `.dylib`) and packages it into a usable Godot addon.

### Build Commands

```bash
# 1. Initialize submodules (if not done)
git submodule update --init --recursive

# 2. Configure (in 'build' directory)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build . --config Release
```

### Installation

After building, the complete addon is assembled in `build/deploy/addons/wfc/`. To install it into a Godot project:

```bash
# Copy the built addon to the Godot project's addons folder
cp -r build/deploy/addons/wfc <path-to-godot-project>/addons/
```

**Note:** The folder name in `deploy` might be `wfc` or `dungeon_generator` depending on the exact CMake configuration. Check `build/deploy/addons/` after building.

## Development Workflow

1.  **Modify C++:** Edit files in `src/` to add features or fix bugs in the generators.
2.  **Rebuild:** Run the build command in the `build/` directory.
3.  **Update Godot:** Copy the updated binary/addon to the Godot project.
    *   *Tip:* If working directly in a Godot project folder, you can configure CMake to output directly to that project's `addons` folder, or use a symlink for faster iteration.
4.  **Test:** Use the `DungeonPreview` node in Godot or run GDScript unit tests to verify changes.

## Key Classes (Exposed to Godot)

| Class | Description |
| :--- | :--- |
| `BSPDungeonGenerator` | Generates rooms and corridors using BSP. |
| `WalkerDungeonGenerator` | Generates organic shapes/caves using random walkers. |
| `GDTilingWFCv2` | Main Tiling WFC generator (v2 API). |
| `WFCConfiguration` | Reusable configuration for WFC (tiles, rules, weights). |
| `WFCResult` | Result object containing generated data (tiles, walls, floors). |
| `DungeonPreview` | A Godot Node helper for visualizing results in the editor. |

## Conventions

- **Code Style:** Follows standard C++ conventions.
- **Godot API:** Uses `godot-cpp` bindings. Ensure all new classes are properly registered in `src/register_types.cpp`.
- **Submodules:** Always ensure submodules (`godot-cpp`, `fast-wfc`) are up to date before building.
