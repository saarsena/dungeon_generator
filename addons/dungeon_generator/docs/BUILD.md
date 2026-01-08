# Building GDTilingWFC v2

This guide explains how to build the improved GDTilingWFC v2 extension with BSP, Walker, and WFC systems.

## Prerequisites

1. **Build Tools:**
   - CMake 3.9 or higher
   - C++17 compatible compiler (MSVC 2019+ on Windows, GCC 9+/Clang on Linux/Mac)
   - Git (for submodules)

2. **Godot 4.x** (for testing)

## Build Steps

The project builds all systems (WFC, BSP, Walker) together using CMake.

### 1. Initialize Submodules

```bash
cd godot-wfc
git submodule update --init --recursive
```

This downloads:
- `godot-cpp/`: Godot C++ bindings
- `tiling-wfc/`: Core WFC algorithm library

### 2. Configure with CMake

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### 3. Build

```bash
# For Debug build
cmake --build . --config Debug

# For Release build
cmake --build . --config Release
```

### 4. Deploy to Your Project

The build system automatically creates a **deploy directory** with everything you need:

**Deploy location:** `build/deploy/addons/wfc/`

This directory contains:
- All compiled binaries in `bin/{platform}/`
- All GDScript files (plugin.gd, DungeonPreview.gd)
- Configuration files (plugin.cfg, wfc.gdextension)

**To install in your Godot project:**

```bash
# Copy the entire deploy folder to your project
cp -r build/deploy/addons/wfc <your-godot-project>/addons/

# Example for specific project
cp -r build/deploy/addons/wfc /c/dev/my-game/addons/
```

After building, you'll see:
```
==========================================
Build complete! Copy to your project:
  FROM: build/deploy/addons/wfc
  TO:   <your-project>/addons/wfc
==========================================
```

### 5. Available Classes in Godot

After installation, these classes are available in GDScript:

**WFC Classes:**
- `GDTilingWFC` (v1)
- `GDTilingWFCv2` (v2 - recommended)
- `WFCConfiguration` (v2)
- `WFCResult` (v2)

**Dungeon Generators:**
- `BSPDungeonGenerator` - Binary space partitioning (rectangular rooms + corridors)
- `BSPResult` - BSP generation results
- `WalkerDungeonGenerator` - Random walk cave generation
- `WalkerResult` - Walker generation results

**Editor Tools:**
- `DungeonPreview` - Visual node for dungeon preview and testing

## Platform-Specific Instructions

### Windows (MSVC)

```bash
# Install Visual Studio 2019+ with C++ tools
# Install CMake 3.9+

cd godot-wfc
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Copy to your project
cp -r deploy/addons/wfc /c/dev/your-project/addons/
```

**Output:** `build/deploy/addons/wfc/bin/win64/libgodot_wfc.windows.template_release.x86_64.dll`

### Linux

```bash
sudo apt install build-essential cmake git
cd godot-wfc
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Copy to your project
cp -r deploy/addons/wfc ~/your-project/addons/
```

**Output:** `build/deploy/addons/wfc/bin/linux/libgodot_wfc.linux.template_release.x86_64.so`

### macOS

```bash
brew install cmake git
cd godot-wfc
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Copy to your project
cp -r deploy/addons/wfc ~/your-project/addons/
```

**Output:** `build/deploy/addons/wfc/bin/macos/libgodot_wfc.macos.template_release.universal.dylib`

## Testing the Build

### 1. Verify Classes are Registered

In Godot, create a test script:

```gdscript
extends Node

func _ready():
    # Test v2 classes exist
    var config = WFCConfiguration.new()
    print("WFCConfiguration: ", config != null)

    var wfc = GDTilingWFCv2.new()
    print("GDTilingWFCv2: ", wfc != null)

    var result = WFCResult.new()
    print("WFCResult: ", result != null)

    # Test basic functionality
    config.enable_connection_system()
    config.add_connected_tile(0, {"left": true}, 1.0)
    print("Tile count: ", config.get_tile_count())
```

### 2. Run Example Dungeon Generation

Use the provided example:

```gdscript
# Load the example
var example = preload("res://elements/dungeon/wfc_dungeon_v2_example.gd").new()

# Generate a simple dungeon
var result = example.generate_simple_dungeon(20, 20)

if result.is_success():
    print("Success! WFC size: ", result.get_wfc_width(), "x", result.get_wfc_height())
    print("Floor count: ", result.get_floor_positions().size())
else:
    print("Failed: ", result.get_failure_reason())
```

### 3. Run Full Stamped Dungeon Test

```gdscript
var result = example.generate_stamped_dungeon(20, 20)

if result.is_success():
    print("WFC grid: ", result.get_wfc_width(), "x", result.get_wfc_height())
    print("Expanded: ", result.get_expanded_width(), "x", result.get_expanded_height())
    print("Floors: ", result.get_floor_positions(0).size())
    print("Walls: ", result.get_wall_positions(1).size())
    print("Distribution: ", result.get_tile_distribution())
```

## Troubleshooting

### Build Errors

**"Cannot find godot-cpp":**
```bash
git submodule update --init --recursive
```

**"Symbol not found" errors:**
- C++17 is enabled by default in CMakeLists.txt
- If issues persist, verify your compiler supports C++17

**"Cannot find tiling-wfc headers":**
- Verify submodule is initialized: `ls tiling-wfc/include/`

### Runtime Errors

**"Class WFCResult not found":**
- Verify registration file is compiled
- Check `addons/wfc/wfc.gdextension` points to correct binary
- Restart Godot editor

**"WFC failed - no valid solution":**
- Check neighbor rules are valid
- Try with debug mode: `wfc.enable_debug(true)`
- Increase grid size
- Adjust tile weights

**Stamps not expanding:**
- Verify all tiles have stamps: `config.set_tile_stamp(...)`
- Check stamp size is set: `config.set_stamp_size(3)`
- Ensure stamp patterns are correct size (3x3 = 9 values)

## Performance Benchmarks

After building, you can benchmark against v1:

```gdscript
var iterations = 100
var start_time = Time.get_ticks_msec()

for i in range(iterations):
    var wfc = GDTilingWFCv2.new(cached_config)  # Reuse config!
    wfc.set_size(20, 20)
    wfc.set_seed(i)
    var result = wfc.run()

var elapsed = Time.get_ticks_msec() - start_time
print("v2 with reused config: ", elapsed, "ms for ", iterations, " generations")
print("Average: ", elapsed / float(iterations), "ms per dungeon")
```

Expected results:
- **v1:** ~50ms per generation × 100 = 5000ms total
- **v2 (reused config):** ~5ms per generation × 100 = 500ms total
- **Speedup:** 10x faster

## Next Steps

After successful build and testing:

1. ✅ Update your dungeon generator to use v2 API
2. ✅ Create reusable configurations for different dungeon types
3. ✅ Benchmark performance improvements
4. ✅ Share with roguelike community!

## Support

If you encounter issues:
1. Check this guide's troubleshooting section
2. Review `WFC_V2_IMPROVEMENTS.md` for API documentation
3. Examine `wfc_dungeon_v2_example.gd` for usage examples
4. File an issue with:
   - Build command used
   - Full error output
   - Platform/compiler version
