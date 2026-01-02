# DungeonPreview

**Inherits:** Node2D

Visual editor tool and runtime dungeon generator supporting three algorithms: BSP, Walker, and WFC.

## Description

DungeonPreview is a versatile Node2D that serves two purposes:

1. **Editor Tool**: Visual preview of dungeon generation with real-time parameter tuning
2. **Runtime Generator**: Production-ready dungeon generation with automatic TileMapLayer population

The node provides a unified interface for all three dungeon generation algorithms, making it easy to experiment and compare different approaches.

## Algorithms

### BSP (Binary Space Partitioning)

Creates rectangular rooms connected by L-shaped corridors. Best for traditional roguelike dungeons.

**Parameters:**
- `bsp_map_width` / `bsp_map_height` - Dungeon dimensions
- `bsp_min_room_size` / `bsp_max_room_size` - Room size range
- `bsp_max_splits` - Number of partitions (more splits = more rooms)
- `bsp_room_padding` - Space between rooms and partition edges
- `bsp_use_seed` / `bsp_seed` - Deterministic generation

### Walker

Creates organic, cave-like dungeons using random walk algorithm. Best for natural formations.

**Parameters:**
- `walker_total_floor_count` - Total floor tiles to generate
- `walker_allow_overlap` - Whether paths can overlap
- `walker_min_hall` / `walker_max_hall` - Hallway length range
- `walker_room_dim` - Square room dimension
- `walker_use_seed` / `walker_seed` - Deterministic generation

### WFC (Wave Function Collapse)

Generates dungeons based on tile constraints and patterns. Best for complex tile-based layouts.

**Parameters:**
- `wfc_width` / `wfc_height` - Grid dimensions
- `wfc_floor_weight` - Probability weight for floor tiles
- `wfc_horizontal_corridor_weight` - Weight for horizontal corridors
- `wfc_vertical_corridor_weight` - Weight for vertical corridors
- `wfc_cross_weight` - Weight for crossroads
- `wfc_use_seed` / `wfc_seed` - Deterministic generation

## Properties

### Generator

| Type | Property | Default | Description |
|------|----------|---------|-------------|
| Algorithm | algorithm | WALKER | Selected algorithm (WFC, WALKER, or BSP) |
| bool | generate | false | Trigger generation (auto-resets to false) |

### TileMapLayer Integration

| Type | Property | Default | Description |
|------|----------|---------|-------------|
| TileMapLayer | target_tilemap_layer | null | Target layer to populate |
| int | floor_tile_source_id | 0 | Tileset source ID for floors |
| Vector2i | floor_tile_atlas_coords | (0,0) | Atlas coords for floors |
| int | wall_tile_source_id | 0 | Tileset source ID for walls |
| Vector2i | wall_tile_atlas_coords | (1,0) | Atlas coords for walls |
| bool | auto_populate_tilemap | true | Auto-populate on generation |

### Display Settings

| Type | Property | Default | Description |
|------|----------|---------|-------------|
| Vector2 | tile_size | (16,16) | Visual tile size in pixels |
| Color | floor_color | Gray | Floor display color |
| Color | wall_color | Dark purple | Wall display color |
| Color | background_color | Dark gray | Background color |
| bool | center_view | true | Center the dungeon in view |

### Grid Overlay

| Type | Property | Default | Description |
|------|----------|---------|-------------|
| bool | show_grid | false | Display grid lines |
| Color | grid_color | Transparent gray | Grid line color |
| bool | show_coordinates | false | Show coordinate labels |
| int | coordinate_interval | 10 | Spacing between labels |

### Statistics

| Type | Property | Default | Description |
|------|----------|---------|-------------|
| bool | show_statistics | true | Display generation stats |
| Vector2 | stats_position | (10,10) | Stats overlay position |
| bool | stats_background | true | Draw background behind stats |
| int | stats_font_size | 14 | Font size for stats |

### Performance

| Type | Property | Default | Description |
|------|----------|---------|-------------|
| bool | enable_lod | true | Level of detail for large dungeons |
| int | lod_threshold | 2000 | Tile count for LOD activation |
| int | max_visible_tiles | 10000 | Maximum tiles to draw |

## Methods

### generate_dungeon() -> void

Generate a dungeon using the selected algorithm and current settings.

This is the main entry point for dungeon generation. It will:
1. Create the appropriate generator (Walker, WFC, or BSP)
2. Configure it with the current settings
3. Run the generation algorithm
4. Display the result visually
5. Populate the target TileMapLayer if configured

**Example:**
```gdscript
$DungeonPreview.algorithm = DungeonPreview.Algorithm.BSP
$DungeonPreview.generate_dungeon()
```

### populate_tilemap() -> void

Populate the target TileMapLayer with generated dungeon tiles.

This method writes the generated floor and wall positions to the configured TileMapLayer using the specified tile IDs and atlas coordinates.

**Requires:**
- `target_tilemap_layer` to be assigned
- A dungeon to be generated first (floor_positions not empty)

**Example:**
```gdscript
$DungeonPreview.target_tilemap_layer = $TileMapLayer
$DungeonPreview.generate_dungeon()
# If auto_populate_tilemap is false:
$DungeonPreview.populate_tilemap()
```

### display_result(result: RefCounted, slot: int = 0) -> void

Display a generation result (WalkerResult, WFCResult, or BSPResult).

Used internally but can be called manually to display custom results.

### set_floor_positions(positions: PackedVector2Array) -> void

Set floor positions directly.

### set_wall_positions(positions: PackedVector2Array) -> void

Set wall positions directly.

### clear() -> void

Clear all displayed data and reset the preview.

### export_as_png(path: String = "") -> int

Export the current preview as a PNG image.

## Usage Examples

### Editor Preview

1. Add DungeonPreview node to your scene
2. In the Inspector, select an algorithm
3. Adjust algorithm-specific parameters
4. Check the "Generate" box to preview
5. Tweak parameters and regenerate until satisfied

### Runtime Generation (Simple)

```gdscript
extends Node2D

@onready var dungeon = $DungeonPreview
@onready var tilemap = $TileMapLayer

func _ready():
    # Configure
    dungeon.algorithm = DungeonPreview.Algorithm.BSP
    dungeon.bsp_max_splits = 7
    dungeon.target_tilemap_layer = tilemap

    # Generate and auto-populate
    dungeon.generate_dungeon()
```

### Runtime Generation (Multi-Floor)

```gdscript
extends Node2D

@onready var dungeon = $DungeonPreview
@onready var tilemap = $TileMapLayer

func generate_floor(floor_number: int):
    # Use floor number as seed for consistency
    dungeon.algorithm = DungeonPreview.Algorithm.WALKER
    dungeon.walker_use_seed = true
    dungeon.walker_seed = floor_number
    dungeon.walker_total_floor_count = 300 + (floor_number * 50)
    dungeon.target_tilemap_layer = tilemap
    dungeon.generate_dungeon()

func _ready():
    generate_floor(1)

func _on_next_floor_pressed():
    current_floor += 1
    tilemap.clear()
    generate_floor(current_floor)
```

### Comparing Algorithms

```gdscript
func compare_algorithms():
    var algorithms = [
        DungeonPreview.Algorithm.BSP,
        DungeonPreview.Algorithm.WALKER,
        DungeonPreview.Algorithm.WFC
    ]

    for algo in algorithms:
        dungeon.algorithm = algo
        dungeon.generate_dungeon()
        await get_tree().create_timer(2.0).timeout
```

### Custom Tile Configuration

```gdscript
func setup_fancy_tiles():
    dungeon.target_tilemap_layer = $TileMapLayer

    # Use different tiles for floors vs walls
    dungeon.floor_tile_source_id = 0
    dungeon.floor_tile_atlas_coords = Vector2i(3, 1)

    dungeon.wall_tile_source_id = 0
    dungeon.wall_tile_atlas_coords = Vector2i(5, 2)

    dungeon.auto_populate_tilemap = true
    dungeon.generate_dungeon()
```

## Performance Tips

1. **Use LOD for large dungeons**: Enable `enable_lod` for dungeons with >2000 tiles
2. **Disable preview in production**: Set `remove_self_on_ready = true` to auto-cleanup
3. **Use direct generators for maximum speed**: For runtime-only generation, use BSPDungeonGenerator/WalkerDungeonGenerator directly instead of DungeonPreview
4. **Cache seeds**: Store seeds for levels you want to regenerate identically

## Visual Customization

```gdscript
func make_it_pretty():
    dungeon.tile_size = Vector2(32, 32)  # Larger tiles
    dungeon.floor_color = Color(0.8, 0.7, 0.6)  # Sandy floor
    dungeon.wall_color = Color(0.3, 0.2, 0.1)  # Dark brown walls
    dungeon.show_grid = true
    dungeon.show_statistics = true
```

## See Also

- [BSPDungeonGenerator](BSPDungeonGenerator.md) - Direct BSP API
- [WalkerDungeonGenerator](WalkerDungeonGenerator.md) - Direct Walker API
- [GDTilingWFCv2](GDTilingWFCv2.md) - Direct WFC API
