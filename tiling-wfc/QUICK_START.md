# Quick Start Guide

Get up and running with the Tiling WFC library in 5 minutes!

## Build and Test (Linux/macOS/Windows)

```bash
cd tiling-wfc
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run the example
./example/Release/simple_example     # Windows
# or
./example/simple_example             # Linux/macOS
```

## Minimal Code Example

```cpp
#include "tiling_wfc.hpp"

int main() {
    // 1. Create tiles
    Array2D<int> tile0(1, 1, 0);  // Tile with value 0
    Array2D<int> tile1(1, 1, 1);  // Tile with value 1

    std::vector<Tile<int>> tiles = {
        Tile<int>(tile0, Symmetry::X, 1.0),
        Tile<int>(tile1, Symmetry::X, 1.0)
    };

    // 2. Define which tiles can be neighbors
    std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>> neighbors = {
        {0, 0, 0, 0},  // tile0 next to tile0
        {0, 0, 1, 0},  // tile0 next to tile1
        {1, 0, 0, 0},  // tile1 next to tile0
        {1, 0, 1, 0}   // tile1 next to tile1
    };

    // 3. Configure options
    TilingWFCOptions options;
    options.periodic_output = false;

    // 4. Create WFC and run
    TilingWFC<int> wfc(tiles, neighbors, 10, 10, options, 12345);
    auto result = wfc.run();

    // 5. Use result
    if (result.has_value()) {
        Array2D<int> output = result.value();
        // Access output data: output.get(y, x)
    }
}
```

## Understanding the Parameters

### Tile Format: `(tile_id, orientation, tile_id, orientation)`
The neighbor rules define spatial relationships:

```
  [tile1]
     |
  [tile2]--[tile3]
```

To say "tile2 can be to the right of tile1":
```cpp
neighbors.push_back({1, 0, 2, 0});  // {tile1, orientation, tile2, orientation}
```

### Symmetry Types

| Symmetry | Orientations | Use Case |
|----------|--------------|----------|
| `X` | 1 | Symmetric in all directions |
| `I` | 2 | Symmetric along one axis (e.g., ─) |
| `backslash` | 2 | Diagonal symmetry (e.g., ╲) |
| `T` | 4 | T-shaped tiles |
| `L` | 4 | L-shaped corners |
| `P` | 8 | Fully asymmetric |

### Weight
Higher weight = more likely to appear in output:
```cpp
Tile<int>(rare_tile, Symmetry::X, 0.1);    // Rare
Tile<int>(common_tile, Symmetry::X, 5.0);  // Common
```

### Periodic Output
```cpp
options.periodic_output = true;   // Wraps around (tileable)
options.periodic_output = false;  // Has edges
```

## Common Patterns

### Simple Maze Generator
```cpp
// 0 = path, 1 = wall
// Ensure paths connect properly
neighbors = {
    {0, 0, 0, 0},  // path-path
    {0, 0, 1, 0},  // path-wall
    {1, 0, 0, 0},  // wall-path
    {1, 0, 1, 0}   // wall-wall
};
```

### Checkerboard Constraint
```cpp
// Force alternating pattern
neighbors = {
    {0, 0, 1, 0},  // 0 only next to 1
    {1, 0, 0, 0}   // 1 only next to 0
};
```

### One-Way Propagation
```cpp
// Tile 1 can be right of tile 0, but not vice versa
neighbors = {
    {0, 0, 1, 0}   // Only this direction
};
```

## Troubleshooting

### ❌ Algorithm Returns `std::nullopt`
**Problem:** No valid solution found (contradiction)

**Solutions:**
1. Make neighbor rules more permissive
2. Reduce output dimensions
3. Try a different seed
4. Check if tile set is self-consistent

### ❌ Output Looks Random/Unstructured
**Problem:** Rules too permissive

**Solutions:**
1. Add more specific neighbor constraints
2. Use different tile symmetries
3. Adjust tile weights

### ❌ Output Too Uniform
**Problem:** Rules too restrictive or weights unbalanced

**Solutions:**
1. Add more tile variety
2. Adjust weights for diversity
3. Use larger output dimensions

## Next Steps

1. **Read the full [README.md](README.md)** for detailed API documentation
2. **Check [GODOT_INTEGRATION.md](GODOT_INTEGRATION.md)** for Godot/GDExtension usage
3. **Experiment** with different tile sets and constraints
4. **Study** the example code in `example/simple_example.cpp`

## File Structure Reference

```
tiling-wfc/
├── include/             # All headers (add to include path)
│   ├── tiling_wfc.hpp  # Main class - start here
│   ├── wfc.hpp         # Core algorithm
│   └── utils/          # Utility classes
├── src/                # Implementation files
├── example/            # Working examples
├── build/              # Build output (gitignored)
├── README.md           # Full documentation
├── QUICK_START.md      # This file
└── GODOT_INTEGRATION.md # Godot guide
```

## Getting Help

- **Issues with building?** Check CMake version (need 3.9+) and C++17 support
- **Questions about algorithm?** See original [fast-wfc](https://github.com/math-fehr/fast-wfc)
- **Want to contribute?** PRs welcome!

Happy generating! 🎲
