# DungeonPreview Examples

This folder contains example scripts demonstrating how to use the dungeon generation system with the DungeonPreview helper node.

## Quick Start

1. **Create a new scene** in Godot
2. **Add a DungeonPreview node** to the scene (Node2D → DungeonPreview)
3. **Attach the example script** `walker_preview_example.gd` to the root node
4. **Toggle "Generate" in the inspector** to see a dungeon appear!

## Example Script Features

### `walker_preview_example.gd`

Demonstrates:
- Generating dungeons with `WalkerDungeonGenerator`
- Generating dungeons with `GDTilingWFCv2`
- Visualizing results with `DungeonPreview`
- Comparing algorithms side-by-side
- Exporting previews as PNG
- Populating a TileMap from generated data

### Inspector Controls

**Generator Settings:**
- `algorithm`: Choose "Walker" or "WFC"
- `allow_overlap`: Walker setting for organic vs structured dungeons
- `total_floor_count`: Target number of floor tiles
- `min_hall` / `max_hall`: Walker corridor length range
- `room_dim`: Walker room size
- `use_seed` / `seed_value`: For reproducible results

**Actions:**
- `generate`: Generate a single dungeon
- `generate_comparison`: Generate Walker vs WFC side-by-side
- `export_png`: Save current preview as PNG
- `clear_preview`: Clear the preview

## Usage Examples

### Example 1: Simple Generation

```gdscript
var walker = WalkerDungeonGenerator.new()
walker.total_floor_count = 300
walker.allow_overlap = true

var result = walker.generate()

$DungeonPreview.display_result(result)
```

### Example 2: Populate TileMap

```gdscript
var walker = WalkerDungeonGenerator.new()
walker.total_floor_count = 500
var result = walker.generate()

var floors = result.get_floor_positions()
var walls = result.get_wall_positions()

for pos in floors:
    $TileMap.set_cell(0, Vector2i(pos), 0)  # Floor tile
for pos in walls:
    $TileMap.set_cell(0, Vector2i(pos), 1)  # Wall tile
```

### Example 3: Compare Algorithms

```gdscript
# Generate Walker dungeon
var walker = WalkerDungeonGenerator.new()
walker.total_floor_count = 300
var walker_result = walker.generate()

# Generate WFC dungeon
var wfc = GDTilingWFCv2.new()
wfc.use_connection_system()
wfc.add_connected_tile(0, {"borders_all": true}, 1.0)
wfc.auto_generate_rules()
wfc.set_size(20, 20)
var wfc_result = wfc.run()

# Display side-by-side
$DungeonPreview.split_view_enabled = true
$DungeonPreview.display_result(walker_result, 0)  # Left side
$DungeonPreview.display_result(wfc_result, 1)     # Right side
```

### Example 4: Procedural Dungeon Floors

```gdscript
# Generate different dungeon for each floor
func enter_floor(floor_number: int):
    var walker = WalkerDungeonGenerator.new()
    walker.use_seed = true
    walker.seed = floor_number  # Different seed per floor
    walker.total_floor_count = 200 + floor_number * 50  # Larger floors deeper

    var result = walker.generate()
    populate_tilemap($TileMap, result)
    spawn_enemies(result.get_floor_positions())
```

### Example 5: Export Preview

```gdscript
# Generate and export multiple variations
for i in range(10):
    var walker = WalkerDungeonGenerator.new()
    walker.seed = i
    walker.use_seed = true
    var result = walker.generate()

    $DungeonPreview.display_result(result)
    $DungeonPreview.export_as_png("res://exports/dungeon_%d.png" % i)
```

## DungeonPreview Features

### Display Settings
- Tile size
- Floor/wall/background colors
- View centering

### Grid Overlay
- Toggle grid lines
- Show/hide coordinates
- Coordinate interval

### Statistics
- Floor/wall counts
- Map dimensions
- Generation time
- Custom statistics from generators

### Comparison Mode
- Split view (horizontal/vertical)
- Compare different algorithms
- Compare different parameters

### Performance
- LOD (Level of Detail) for large dungeons
- Max visible tiles limit
- Automatic culling

### Export
- Save as PNG
- Configurable scale
- Auto-crops to dungeon bounds

### Safety
- `remove_self_on_ready` prevents shipping debug nodes

## Tips

1. **Enable statistics** to see generation performance
2. **Use grid overlay** when debugging tile placement
3. **Enable split view** to compare algorithm outputs
4. **Export previews** to share designs with team
5. **Set remove_self_on_ready** to true before building final game
6. **Use LOD** for dungeons with 2000+ tiles for smooth editor performance

## See Also

- `WalkerDungeonGenerator` - Fast procedural walker algorithm
- `GDTilingWFCv2` - Constraint-based Wave Function Collapse
- (Future) `BSPGenerator` - Binary Space Partitioning
