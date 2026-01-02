# Tiling WFC Library

A minimal, standalone C++17 library containing only the **Tiling WFC (Wave Function Collapse)** algorithm extracted from [fast-wfc](https://github.com/math-fehr/fast-wfc).

This library is designed to be lightweight and easy to integrate, making it perfect for embedding in other projects or wrapping with GDExtension for Godot.

## Features

- **Header-mostly design** - Easy to integrate
- **No external dependencies** - Only uses C++ standard library
- **Clean API** - Simple tile-based constraint system
- **CMake support** - Easy to build and integrate
- **Static and shared library options**

## Library Structure

```
tiling-wfc/
├── include/
│   ├── tiling_wfc.hpp      # Main TilingWFC class
│   ├── wfc.hpp              # Generic WFC algorithm
│   ├── propagator.hpp       # Pattern propagation
│   ├── wave.hpp             # Wave state management
│   ├── direction.hpp        # Direction utilities
│   └── utils/
│       ├── array2D.hpp      # 2D array container
│       └── array3D.hpp      # 3D array container
├── src/
│   ├── wfc.cpp              # WFC implementation
│   ├── propagator.cpp       # Propagator implementation
│   └── wave.cpp             # Wave implementation
├── example/
│   └── simple_example.cpp   # Usage example
└── CMakeLists.txt           # Build configuration
```

## Building

### Requirements
- CMake 3.9 or higher
- C++17 compatible compiler

### Build Instructions

```bash
cd tiling-wfc
mkdir build
cd build
cmake ..
cmake --build .
```

This will create:
- `libtiling_wfc.a` (static library)
- `libtiling_wfc.so` / `tiling_wfc.dll` (shared library)
- `simple_example` executable (if `BUILD_EXAMPLE=ON`)

### CMake Options

- `BUILD_EXAMPLE` - Build the example application (default: ON)
- `CMAKE_BUILD_TYPE` - Build type: Release/Debug (default: Release)

## Usage

### Basic Example

```cpp
#include "tiling_wfc.hpp"

// Create tiles
Array2D<int> tile_data(1, 1, 0);
std::vector<Tile<int>> tiles = {
    Tile<int>(tile_data, Symmetry::X, 1.0)
};

// Define neighbor rules (which tiles can be adjacent)
std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>> neighbors = {
    std::make_tuple(0, 0, 0, 0)  // tile 0 can be next to tile 0
};

// Configure options
TilingWFCOptions options;
options.periodic_output = false;

// Create and run WFC
TilingWFC<int> wfc(tiles, neighbors, height, width, options, seed);
auto result = wfc.run();

if (result.has_value()) {
    Array2D<int> output = result.value();
    // Use the generated output
}
```

### Key Concepts

1. **Tiles**: Define your tile set with symmetry and weight
   - `Symmetry::X` - No rotation (1 orientation)
   - `Symmetry::I` / `backslash` - 2 orientations
   - `Symmetry::T` / `L` - 4 orientations
   - `Symmetry::P` - 8 orientations (all rotations + reflections)

2. **Neighbors**: Define adjacency rules between tiles
   - Format: `(tile1_id, tile1_orientation, tile2_id, tile2_orientation)`
   - Specifies which tiles can be placed next to each other

3. **Options**:
   - `periodic_output` - Whether the output wraps around (toric topology)

4. **Run**: Execute the algorithm
   - Returns `std::optional<Array2D<T>>`
   - Returns `std::nullopt` if no valid solution found

## Integration with Godot (GDExtension)

This library is specifically designed to be wrapped with GDExtension for Godot. The minimal dependencies and clean API make it ideal for:

1. Link the static library in your GDExtension project
2. Expose the `TilingWFC` class to Godot
3. Use Godot's resource system for tile definitions
4. Generate procedural tilemaps at runtime

Example integration structure:
```
godot-wfc-extension/
├── tiling-wfc/           # This library as submodule
└── src/
    └── gdwfc.cpp         # GDExtension wrapper
```

## API Reference

### TilingWFC Class

**Constructor:**
```cpp
TilingWFC(
    const std::vector<Tile<T>> &tiles,
    const std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>> &neighbors,
    unsigned height,
    unsigned width,
    const TilingWFCOptions &options,
    int seed
)
```

**Methods:**
- `std::optional<Array2D<T>> run()` - Execute the algorithm
- `bool set_tile(unsigned tile_id, unsigned orientation, unsigned i, unsigned j)` - Constrain a specific position

### Tile Structure

```cpp
template <typename T>
struct Tile {
    std::vector<Array2D<T>> data;  // Tile orientations
    Symmetry symmetry;              // Symmetry type
    double weight;                  // Probability weight
};
```

## License

This library maintains the same license as the original fast-wfc project.

## Credits

Extracted from [fast-wfc](https://github.com/math-fehr/fast-wfc) by Mathieu Fehr.
