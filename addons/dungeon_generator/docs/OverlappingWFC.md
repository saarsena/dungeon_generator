# Overlapping WFC Implementation Guide

## Overview

The Overlapping WFC system allows you to generate dungeons and levels by learning patterns from a seed image. Unlike Tiling WFC which uses pre-defined tiles and rules, Overlapping WFC analyzes an example image and generates similar-looking outputs.
/home/saarsena/Pictures/Screenshots/20260108_162113.png
## Key Features

1. **Pattern Learning**: Extracts small NxN patterns from a seed image
2. **Stamp System Integration**: Like Tiling WFC, supports expanding patterns to detailed tiles
3. **Inspector Integration**: Full Godot editor support with live preview
4. **Color-to-Tile Mapping**: Convert pattern colors to tile IDs for stamp expansion
5. **Export Support**: Export to PNG or TileMapLayer

## Architecture

### C++ Classes

#### `OverlappingWFCGenerator`
Main generation class that:
- Loads seed images from Godot `Image` resources
- Configures WFC parameters (pattern size, symmetry, etc.)
- Manages pattern-to-tile mappings
- Manages stamp patterns for tile expansion
- Runs the WFC algorithm

#### `OverlappingWFCResult`
Result container that stores:
- Raw pattern output from WFC
- Tile-mapped output (if mappings configured)
- Expanded output with stamps (if enabled)
- Helper methods for getting floor/wall positions

### GDScript Node

#### `OverlappingWFCPreview`
Editor preview node that provides:
- Load seed images in inspector
- Configure all WFC parameters
- Live preview in editor
- Statistics overlay
- Export to PNG or TileMapLayer

## Usage

### Basic Usage

```gdscript
# Create generator
var generator = OverlappingWFCGenerator.new()

# Load seed image
var seed_img = Image.load_from_file("res://dungeon_seed.png")
generator.set_seed_image(seed_img)

# Configure output
generator.set_output_size(48, 48)
generator.set_pattern_size(3)
generator.set_symmetry(8)

# Generate
var result = generator.generate()

if result.is_success():
    var floors = result.get_floor_positions()
    var walls = result.get_wall_positions()
```

### With Stamp System

```gdscript
var generator = OverlappingWFCGenerator.new()
generator.set_seed_image(seed_img)
generator.set_output_size(48, 48)

# Enable stamps
generator.enable_stamps(true)
generator.set_stamp_size(3)

# Use default dungeon mapping (black=floor, white=wall)
generator.setup_default_dungeon_mapping()
generator.setup_default_dungeon_stamps()

var result = generator.generate()

# Result now has expanded 3x3 stamps applied
var floors = result.get_floor_positions()  # From expanded output
```

### Custom Pattern Mappings

```gdscript
generator.enable_stamps(true)
generator.set_stamp_size(3)

# Map pattern colors to tile IDs
generator.add_pattern_to_tile_mapping(0x000000, 0)  # Black -> Floor (ID 0)
generator.add_pattern_to_tile_mapping(0xFFFFFF, 1)  # White -> Wall (ID 1)
generator.add_pattern_to_tile_mapping(0xFF0000, 2)  # Red -> Lava (ID 2)

# Set custom stamp for each tile ID
var floor_stamp = PackedInt32Array([0,0,0, 0,0,0, 0,0,0])  # 3x3 all floor
generator.set_tile_stamp(0, floor_stamp, 3, 3)

var wall_stamp = PackedInt32Array([1,1,1, 1,1,1, 1,1,1])  # 3x3 all wall
generator.set_tile_stamp(1, wall_stamp, 3, 3)

var lava_stamp = PackedInt32Array([2,2,2, 2,2,2, 2,2,2])  # 3x3 all lava
generator.set_tile_stamp(2, lava_stamp, 3, 3)

var result = generator.generate()
```

## Using OverlappingWFCPreview in Editor

1. **Add Node**: Add `OverlappingWFCPreview` to your scene
2. **Load Seed Image**:
   - Create or import a small PNG image (e.g., 10x10 pixels)
   - Black pixels = floors, White pixels = walls
   - Drag it to the `seed_image` property
3. **Configure Settings**:
   - `output_width/output_height`: Size of generated output
   - `pattern_size`: Size of patterns to extract (2-5, typically 3)
   - `symmetry`: Number of rotations/reflections (1-8, typically 8)
   - `enable_stamps`: Check to expand patterns with stamps
   - `use_default_dungeon_mapping`: Use black/white -> floor/wall mapping
4. **Generate**: Check the `generate` checkbox
5. **View Result**: The preview will update with the generated dungeon

## Workflow: Seed Image Creation

### Simple Dungeon Seed

Create a small 10x10 PNG image:
- Black pixels (#000000) = Floor/walkable space
- White pixels (#FFFFFF) = Wall/obstacles

Example patterns the WFC will learn:
- Corners (L-shapes)
- Corridors (straight lines)
- Rooms (large black areas)
- Wall thickness

The WFC algorithm will:
1. Extract all 3x3 patterns from your seed image
2. Learn which patterns can be adjacent
3. Generate a larger output that looks similar
4. (Optional) Map colors to tile IDs and expand with stamps

### Tips for Good Seed Images

1. **Small but representative**: 10x15 pixels is enough to show variety
2. **Include all patterns**: Make sure your seed includes corners, corridors, rooms
3. **Clear contrast**: Use pure black/white for best results
4. **Tileable**: If using `periodic_input=true`, make edges wrap correctly

## Parameters Reference

### WFC Algorithm Parameters

- **pattern_size** (2-5): Size of patterns to extract. Smaller = more variation, larger = closer to original
- **symmetry** (1-8): Number of pattern orientations. 8 = all rotations and reflections
- **periodic_input**: Treat seed image edges as wrapping
- **periodic_output**: Make output tileable
- **ground_mode**: Pin bottom row to ground pattern

### Stamp System Parameters

- **enable_stamps**: Enable stamp expansion
- **stamp_size** (1-5): Size of each stamp (typically 3 for 3x3)
- **use_default_dungeon_mapping**: Auto-map black=floor(0), white=wall(1)
- **custom_pattern_mappings**: Dictionary of color_value -> tile_id
- **custom_tile_stamps**: Dictionary of tile_id -> stamp data

## Output Structure

The generation produces three levels of output:

1. **Pattern Output** (`get_pattern_output()`):
   - Raw WFC output
   - Each value is a color from the seed image
   - Size: `output_width x output_height`

2. **Tile Output** (`get_tile_output()`):
   - Pattern colors mapped to tile IDs
   - Only available if pattern mappings configured
   - Size: `output_width x output_height`

3. **Expanded Output** (`get_expanded_output()`):
   - Tiles expanded with stamps
   - Only available if stamps enabled
   - Size: `(output_width * stamp_size) x (output_height * stamp_size)`

Helper methods automatically use the most detailed output available:
- `get_floor_positions()` -> Uses expanded if available, otherwise tile, otherwise pattern
- `get_wall_positions()` -> Same logic

## Example Seed Images

### Simple Corridor Dungeon (10x10)
```
⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜
⬜⬛⬛⬛⬜⬛⬛⬛⬛⬜
⬜⬛⬛⬛⬜⬛⬛⬛⬛⬜
⬜⬛⬛⬛⬛⬛⬛⬛⬛⬜
⬜⬜⬜⬛⬛⬛⬜⬜⬜⬜
⬜⬛⬛⬛⬛⬛⬛⬛⬜⬜
⬜⬛⬛⬛⬛⬛⬛⬛⬜⬜
⬜⬛⬛⬛⬜⬜⬜⬜⬜⬜
⬜⬛⬛⬛⬜⬜⬜⬜⬜⬜
⬜⬜⬜⬜⬜⬜⬜⬜⬜⬜

⬛ = Black (#000000) = Floor
⬜ = White (#FFFFFF) = Wall
```

This seed teaches the algorithm:
- Rectangular rooms
- 1-tile wide corridors
- L-shaped corners
- Wall boundaries

## Integration with Existing Systems

The Overlapping WFC system integrates seamlessly with:

1. **DungeonPreview**: Can display results like Walker/BSP generators
2. **Stamp System**: Uses same 3x3 stamp format as Tiling WFC
3. **TileMapLayer**: Can populate TileMaps just like other generators

## Future Enhancements

- [ ] Support for multi-color seed images (more than just black/white)
- [ ] Advanced stamp patterns (5x5, variable sizes)
- [ ] Constraint system (force specific patterns in specific locations)
- [ ] Multi-layer generation (floors + walls + decorations)

## Technical Notes

### Performance

- Pattern extraction is O(w * h * symmetry)
- Generation time depends on output size and pattern_size
- Typical 48x48 output with pattern_size=3: ~50-200ms
- Stamp expansion is fast (simple array copy)

### Dependencies

- Uses `fast-wfc` library for core algorithm
- Requires Godot's `Image` class for seed image loading
- Compatible with Godot 4.x

### Build System

The implementation includes:
- C++ source files in `src/overlapping_wfc_godot.*`
- Registration in `src/register_types.cpp`
- CMake configuration in `CMakeLists.txt`
- fast-wfc library source compiled directly

## Troubleshooting

### Generation Fails (Contradiction)

- **Problem**: WFC cannot find valid solution
- **Solutions**:
  - Increase `output_size` (give more room)
  - Decrease `pattern_size` (less strict constraints)
  - Check seed image has enough variety
  - Try different `symmetry` value

### Output Doesn't Look Like Seed

- **Problem**: Too much variation
- **Solutions**:
  - Increase `pattern_size` (stricter matching)
  - Decrease `symmetry` (fewer transformations)
  - Make seed image larger with more examples

### Stamps Not Appearing

- **Problem**: Expanded output is empty
- **Solutions**:
  - Check `enable_stamps` is true
  - Verify pattern mappings are set
  - Verify tile stamps are set for all tile IDs
  - Check debug output for errors

## Examples in Project

See:
- `addons/dungeon_generator/examples/overlapping_wfc_example.tscn` (when added)
- `fast-wfc/example/samples/Dungeon.png` for seed image example
