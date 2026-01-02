# Integrating Tiling WFC with Godot (GDExtension)

This guide explains how to wrap the Tiling WFC library for use in Godot 4 via GDExtension.

## Overview

The Tiling WFC library is designed to be easily wrapped for Godot. Here's a recommended approach:

## Step 1: Project Structure

```
your-godot-wfc-project/
├── addons/
│   └── wfc/
│       ├── bin/           # Compiled GDExtension binaries
│       └── wfc.gdextension
├── src/
│   ├── gdwfc.h           # GDExtension wrapper header
│   ├── gdwfc.cpp         # GDExtension wrapper implementation
│   └── register_types.cpp
├── tiling-wfc/           # This library (as git submodule)
└── CMakeLists.txt
```

## Step 2: Add as Git Submodule

```bash
cd your-godot-wfc-project
git submodule add <tiling-wfc-repo-url> tiling-wfc
git submodule update --init --recursive
```

## Step 3: Create GDExtension Wrapper Class

### gdwfc.h
```cpp
#ifndef GDWFC_H
#define GDWFC_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include "tiling_wfc.hpp"

using namespace godot;

class GDTilingWFC : public RefCounted {
    GDCLASS(GDTilingWFC, RefCounted)

private:
    int width;
    int height;
    int seed;
    bool periodic;
    // Store tiles and neighbor rules

protected:
    static void _bind_methods();

public:
    GDTilingWFC();
    ~GDTilingWFC();

    void set_size(int p_width, int p_height);
    void set_seed(int p_seed);
    void set_periodic(bool p_periodic);

    // Add tile with symmetry and weight
    void add_tile(int tile_id, TypedArray<int> tile_data, int symmetry, float weight);

    // Add neighbor rule
    void add_neighbor_rule(int tile1_id, int orientation1, int tile2_id, int orientation2);

    // Run the algorithm
    TypedArray<int> run();

    // Get result dimensions
    int get_output_width() const;
    int get_output_height() const;
};

#endif // GDWFC_H
```

### gdwfc.cpp (skeleton)
```cpp
#include "gdwfc.h"
#include <godot_cpp/core/class_db.hpp>

void GDTilingWFC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_size", "width", "height"), &GDTilingWFC::set_size);
    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &GDTilingWFC::set_seed);
    ClassDB::bind_method(D_METHOD("set_periodic", "periodic"), &GDTilingWFC::set_periodic);
    ClassDB::bind_method(D_METHOD("add_tile", "tile_id", "tile_data", "symmetry", "weight"), &GDTilingWFC::add_tile);
    ClassDB::bind_method(D_METHOD("add_neighbor_rule", "tile1_id", "orientation1", "tile2_id", "orientation2"), &GDTilingWFC::add_neighbor_rule);
    ClassDB::bind_method(D_METHOD("run"), &GDTilingWFC::run);

    // Bind symmetry constants
    BIND_ENUM_CONSTANT(SYMMETRY_X);      // 0 - No rotation
    BIND_ENUM_CONSTANT(SYMMETRY_I);      // 1 - 2 orientations
    BIND_ENUM_CONSTANT(SYMMETRY_BACKSLASH); // 2
    BIND_ENUM_CONSTANT(SYMMETRY_T);      // 3 - 4 orientations
    BIND_ENUM_CONSTANT(SYMMETRY_L);      // 4
    BIND_ENUM_CONSTANT(SYMMETRY_P);      // 5 - 8 orientations
}

GDTilingWFC::GDTilingWFC() : width(10), height(10), seed(0), periodic(false) {}

GDTilingWFC::~GDTilingWFC() {}

// Implement methods...
```

## Step 4: CMakeLists.txt for GDExtension

```cmake
cmake_minimum_required(VERSION 3.9)
project(godot_wfc)

set(CMAKE_CXX_STANDARD 17)

# Add tiling-wfc library
add_subdirectory(tiling-wfc)

# Godot-cpp (you'll need to add this as submodule too)
add_subdirectory(godot-cpp)

# Your GDExtension
add_library(${PROJECT_NAME} SHARED
    src/gdwfc.cpp
    src/register_types.cpp
)

target_link_libraries(${PROJECT_NAME}
    godot-cpp
    tiling_wfc_static
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    tiling-wfc/include/
)
```

## Step 5: Usage in GDScript

```gdscript
extends Node

func _ready():
    var wfc = GDTilingWFC.new()

    # Configure
    wfc.set_size(20, 20)
    wfc.set_seed(12345)
    wfc.set_periodic(false)

    # Define tiles (simple example with tile IDs 0 and 1)
    # Tile data could be color values, tile indices, etc.
    wfc.add_tile(0, [0], GDTilingWFC.SYMMETRY_X, 1.0)
    wfc.add_tile(1, [1], GDTilingWFC.SYMMETRY_X, 1.0)

    # Define neighbor rules (which tiles can be adjacent)
    wfc.add_neighbor_rule(0, 0, 0, 0)  # Tile 0 can be next to tile 0
    wfc.add_neighbor_rule(0, 0, 1, 0)  # Tile 0 can be next to tile 1
    wfc.add_neighbor_rule(1, 0, 0, 0)  # Tile 1 can be next to tile 0
    wfc.add_neighbor_rule(1, 0, 1, 0)  # Tile 1 can be next to tile 1

    # Run the algorithm
    var result = wfc.run()

    if result:
        print("WFC succeeded!")
        print("Output dimensions: ", wfc.get_output_width(), "x", wfc.get_output_height())
        # Use result to populate a TileMap
        generate_tilemap(result, wfc.get_output_width(), wfc.get_output_height())
    else:
        print("WFC failed to find a solution")

func generate_tilemap(data: Array, width: int, height: int):
    var tilemap = $TileMap
    for y in range(height):
        for x in range(width):
            var tile_id = data[y * width + x]
            tilemap.set_cell(0, Vector2i(x, y), 0, Vector2i(tile_id, 0))
```

## Tips for Integration

### 1. Tile Data Format
- Use Godot's `Image` or `Texture2D` for visual tiles
- Store tile metadata in custom resources
- Consider using `PackedInt32Array` for efficient data transfer

### 2. Resource-Based Tile Definition
Create a Godot Resource for tiles:

```gdscript
class_name WFCTile
extends Resource

@export var tile_id: int
@export var texture: Texture2D
@export var symmetry: int
@export var weight: float = 1.0
```

### 3. Visual Editor
Consider creating an editor plugin to:
- Visually define tiles
- Draw neighbor connections
- Preview WFC results in-editor

### 4. Async Generation
For large outputs, consider:
- Running WFC in a separate thread
- Using Godot's `WorkerThreadPool`
- Showing progress feedback

### 5. Caching
- Cache successful WFC runs
- Store seeds that produce good results
- Pre-generate common patterns

## Performance Considerations

- The library uses `std::optional` which requires C++17
- For best performance, use static linking (`tiling_wfc_static`)
- Consider constraining output size for real-time generation
- Use periodic mode for seamless tiling

## Debugging

Enable detailed logging in your wrapper:
```cpp
if (result.has_value()) {
    godot::UtilityFunctions::print("WFC success!");
} else {
    godot::UtilityFunctions::print("WFC failed - contradiction encountered");
}
```

## Example Projects

Potential use cases in Godot:
- Procedural dungeon generation
- Terrain generation
- Puzzle generation
- Level design tools
- Runtime map expansion

## Resources

- [GDExtension Documentation](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/)
- [godot-cpp Repository](https://github.com/godotengine/godot-cpp)
- Original WFC Algorithm: [mxgmn/WaveFunctionCollapse](https://github.com/mxgmn/WaveFunctionCollapse)
