# Godot Procedural Dungeon Generator

A high-performance Godot 4.x GDExtension providing four powerful procedural generation algorithms. Designed for roguelikes, procedural level generation, and any game requiring fast dungeon creation.

## Overview

This extension provides **four complete generation algorithms** implemented in C++:

- **BSP (Binary Space Partitioning)** - Rectangular rooms with corridors (classic roguelike style)
- **Walker** - Organic, cave-like dungeons using random walk algorithm
- **Tiling WFC (Wave Function Collapse)** - Constraint-based tile placement for complex patterns
- **Overlapping WFC** - Pattern-based generation from example images (NEW!)

All algorithms include:

- Visual preview tool (DungeonPreview node)
- Direct generator classes for runtime use
- Automatic TileMapLayer integration
- Deterministic generation with seed support

### Key Features

- **Fast C++ implementation** with ergonomic GDScript API
- **Connection-based tile system** - Define tiles by edges, automatically generate neighbor rules
- **10x performance improvement** for multiple level generation through configuration caching
- **Native stamp expansion** - Automatic expansion of WFC tiles to NxN actual tiles
- **Rich result objects** - Built-in helpers for floor/wall extraction, statistics, and debugging
- **Reusable configurations** - Define tile sets once, generate hundreds of levels efficiently

## Performance

Real benchmarks on 20×20 grids (expanding to 60×60 with 3×3 stamps):

- **Average generation time:** 13.6ms per dungeon
- **Throughput:** ~73 dungeons/second (4,370 dungeons/minute)
- **100 generations with v2 (cached config):** 1.37 seconds total
- **Success rate:** 100% (with proper tile configuration)

The v2 API with reusable configurations is approximately **10x faster** than v1 for multiple generations, which would take ~5 seconds for the same 100 dungeons due to reconfiguration overhead.

## Quick Start

### 1. Build the Extension

```bash
git clone https://github.com/yourusername/godot-wfc.git
cd godot-wfc
git submodule update --init --recursive

# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

The build system creates a ready-to-use addon in `build/deploy/addons/wfc/` containing:
- Compiled binaries for your platform
- All GDScript files and configuration
- Everything needed to run the extension

### 2. Install in Godot Project

```bash
# Copy the entire addon folder to your Godot project
cp -r build/deploy/addons/wfc <your-project>/addons/
```

After building, you'll see a helpful message showing exactly where to copy from!

### 3. Choose Your Approach

#### Option A: Visual Node (Easiest)

```gdscript
# Add DungeonPreview node to your scene
@onready var dungeon = $DungeonPreview

func _ready():
    dungeon.algorithm = DungeonPreview.Algorithm.BSP
    dungeon.target_tilemap_layer = $TileMapLayer
    dungeon.generate_dungeon()  # Auto-populates tilemap
```

#### Option B: Direct Generators (Maximum Performance)

```gdscript
# BSP - Rectangular rooms
var bsp = BSPDungeonGenerator.new()
bsp.set_map_size(80, 60)
bsp.set_max_splits(7)
var result = bsp.generate()

# Walker - Organic caves
var walker = WalkerDungeonGenerator.new()
walker.set_total_floor_count(500)
var result = walker.generate()

# Use the results
for pos in result.get_floor_positions():
    tilemap.set_cell(Vector2i(pos), floor_tile_id, atlas_coords)
```

### 4. Algorithm Comparison

| Algorithm | Best For | Style | Typical Use Case |
|-----------|----------|-------|------------------|
| **BSP** | Classic roguelikes | Rectangular rooms + corridors | NetHack, Brogue, traditional dungeons |
| **Walker** | Organic levels | Cave-like, flowing | Caves, mines, natural formations |
| **Tiling WFC** | Complex patterns | Constraint-based tiles | Puzzle levels, specific tile patterns |
| **Overlapping WFC** | Style-matching | Pattern extraction from images | Generate levels that match example art |

## Performance Benchmarks

All generators are implemented in C++ for maximum speed:

### BSP Generator

- **Small (64x64)**: 1-2ms
- **Medium (128x128)**: 5-10ms
- **Large (256x256)**: 20-40ms

### Walker Generator

- **1,000 tiles**: 2-3ms
- **10,000 tiles**: 25-27ms
- **100,000 tiles**: 286-302ms

### Tiling WFC Generator

- **20x20 grid**: 13.6ms average
- **With caching**: 10x faster for multiple generations
- **100 dungeons**: 1.37 seconds total

### Overlapping WFC Generator

- **12x12 seed → 20x20 output**: 15ms
- **39x28 seed → 48x48 output**: 335ms
- Automatic color extraction and pattern matching

## Documentation

- [BSPDungeonGenerator](docs/BSPDungeonGenerator.md) - Binary Space Partitioning API
- [WalkerDungeonGenerator](docs/WalkerDungeonGenerator.md) - Random walker API
- [GDTilingWFCv2](docs/GDTilingWFCv2.md) - Tiling Wave Function Collapse API
- [OverlappingWFC](docs/OverlappingWFC.md) - Overlapping WFC with image-based generation
- [DungeonPreview](docs/DungeonPreview.md) - Visual preview node with TileMapLayer integration
- [BUILD](docs/BUILD.md) - Build instructions and troubleshooting

## Complete Example

```gdscript
extends Node2D

func generate_dungeon():
    # Create WFC generator with connection system
    var wfc = GDTilingWFCv2.new()
    wfc.use_connection_system()

    # Define tiles by their edge connections
    wfc.add_connected_tile(5, {"left": true, "right": true}, 1.0)    # Corridor
    wfc.add_connected_tile(14, {"left": true, "up": true,
                               "right": true, "down": true}, 1.5)    # Cross
    wfc.add_connected_tile(15, {"borders_all": true}, 0.8)          # Empty

    # Auto-generate all neighbor rules from connections
    wfc.auto_generate_rules()

    # Generate level
    wfc.set_size(20, 20)
    wfc.set_seed(randi())
    var result = wfc.run()

    # Process results
    if result.is_success():
        var floors = result.get_floor_positions(0)
        var walls = result.get_wall_positions(1)
        populate_tilemap(result)
    else:
        print("Generation failed: ", result.get_failure_reason())
```

### 4. Advanced Usage with Stamps

```gdscript
func generate_stamped_dungeon():
    var wfc = GDTilingWFCv2.new()
    wfc.use_connection_system()

    # Add tile with 3x3 stamp pattern (0=floor, 1=wall)
    wfc.add_connected_tile(5, {"left": true, "right": true}, 1.0)
    wfc.set_tile_stamp(5, [
        1, 1, 1,  # Wall  Wall  Wall
        0, 0, 0,  # Floor Floor Floor
        1, 1, 1   # Wall  Wall  Wall
    ], 3, 3)

    wfc.auto_generate_rules()
    wfc.set_size(20, 20)
    var result = wfc.run()

    # Result is automatically expanded to 60x60 (20 * 3)
    print("Expanded size: ", result.get_expanded_width(), "x", result.get_expanded_height())
```

## API Overview

### Three Main Classes

```gdscript
# 1. WFCConfiguration - Reusable tile definitions
var config = WFCConfiguration.new()
config.enable_connection_system()
config.add_connected_tile(id, connections, weight)
config.set_tile_stamp(id, pattern, width, height)
config.auto_generate_neighbor_rules()

# 2. GDTilingWFCv2 - Main generator
var wfc = GDTilingWFCv2.new(config)  # or new() then configure
wfc.set_size(width, height)
wfc.set_seed(seed)
var result = wfc.run()

# 3. WFCResult - Rich result object
if result.is_success():
    var floors = result.get_floor_positions(0)
    var walls = result.get_wall_positions(1)
    var stats = result.get_tile_distribution()
else:
    print("Error: ", result.get_failure_reason())
```

### Reusable Configurations

```gdscript
# Create configuration once
static var dungeon_config: WFCConfiguration

static func get_dungeon_config() -> WFCConfiguration:
    if not dungeon_config:
        dungeon_config = WFCConfiguration.new()
        dungeon_config.enable_connection_system()
        # Configure tiles once
        dungeon_config.add_connected_tile(5, {"left": true, "right": true})
        # ... add more tiles
        dungeon_config.auto_generate_neighbor_rules()
    return dungeon_config

# Reuse for fast generation
func generate_multiple_levels():
    var config = get_dungeon_config()  # Cached configuration
    for floor in range(100):
        var wfc = GDTilingWFCv2.new(config)
        wfc.set_seed(floor)
        var result = wfc.run()  # Very fast - no reconfiguration
```

## Requirements

- Godot 4.x
- CMake 3.9+
- C++17-capable compiler (MSVC 2019+, GCC 9+, Clang 10+)
- Git (for submodules)

## Additional Resources

- [docs/BUILD.md](docs/BUILD.md) - Detailed build instructions and troubleshooting
- [example_wfc.gd](example_wfc.gd) - Example GDScript usage
- [tiling-wfc/](tiling-wfc/) - Standalone C++ WFC library documentation

## Use Cases

Well-suited for:

- Roguelike dungeon generation
- Procedural level design
- Randomized mazes and layouts
- Tilemap-based games requiring varied environments
- Level editing tools and procedural content pipelines

## Troubleshooting

### Build Issues

- **Missing submodules:** Run `git submodule update --init --recursive`
- **CMake errors:** Ensure CMake 3.9+ is installed and in your PATH
- **Compiler errors:** Verify C++17 compiler support (MSVC 2019+, GCC 9+, Clang 10+)

### Runtime Issues

- **Extension not found:** Restart Godot editor after installing the addon
- **WFC generation fails:** Enable debug mode with `wfc.enable_debug(true)` and check console output
- **Performance issues:** Use v2 API with cached `WFCConfiguration` objects for multiple generations

See [docs/BUILD.md](docs/BUILD.md) for detailed debugging information.

## Project Structure

```text
godot-wfc/
├── src/                         # GDExtension C++ source
│   ├── gdwfc_v2.{h,cpp}        # WFC v2 API
│   ├── bsp_godot.{h,cpp}       # BSP dungeon generator
│   ├── walker.{h,cpp}          # Walker cave generator
│   └── register_types.cpp      # Godot class registration
├── tiling-wfc/                  # WFC algorithm library (submodule)
├── godot-cpp/                   # Godot C++ bindings (submodule)
├── addons/wfc/                  # Source addon files
│   ├── plugin.gd               # Editor plugin
│   ├── plugin.cfg              # Plugin configuration
│   ├── DungeonPreview.gd       # Preview node
│   └── wfc.gdextension         # Extension definition
├── build/                       # CMake build output
│   └── deploy/                 # Ready-to-copy addon
│       └── addons/wfc/         # Complete addon with binaries
├── docs/                        # Documentation
│   ├── BUILD.md                # Build instructions
│   └── BSPDungeonGenerator.md  # API docs
└── CMakeLists.txt              # CMake build configuration
```

## Contributing

Contributions are welcome:

- Bug reports with minimal reproducible examples
- Focused pull requests for specific improvements
- For major changes, please open an issue for discussion first
- Test changes against provided examples before submitting

## License

MIT License - See [LICENSE](LICENSE) for details.

### Dependencies

- [fast-wfc](https://github.com/math-fehr/fast-wfc) by Mathieu Fehr (MIT License)
- [godot-cpp](https://github.com/godotengine/godot-cpp) by Godot Engine contributors (MIT License)

### Credits

- WFC Algorithm: Maxim Gumin ([WaveFunctionCollapse](https://github.com/mxgmn/WaveFunctionCollapse))
- Fast WFC Implementation: Mathieu Fehr ([fast-wfc](https://github.com/math-fehr/fast-wfc))
- Godot Integration: Project contributors
