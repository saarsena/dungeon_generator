# WalkerDungeonGenerator

A procedural dungeon generator using the "drunken walk" algorithm. Creates organic, cave-like dungeons with rooms and corridors.

## Overview

The Walker generator creates dungeons by simulating one or more "walkers" that randomly move around a grid, carving out floor tiles as they go. It supports two modes:
- **Overlap mode**: Multiple walkers create organic, cave-like structures
- **Non-overlap mode**: Single walker with rectangular rooms connected by corridors

## Classes

### WalkerDungeonGenerator

Main generator class that creates the dungeon.

#### Constructor

```gdscript
var walker = WalkerDungeonGenerator.new()
```

#### Configuration Methods

##### Basic Settings

```gdscript
# Allow rooms and corridors to overlap (creates organic caves)
walker.set_allow_overlap(true)  # default: false

# Total number of floor tiles to generate
walker.set_total_floor_count(300)  # default: 200

# Set random seed for reproducible generation
walker.set_use_seed(true)
walker.set_seed(12345)
```

##### Corridor Settings (Non-overlap mode only)

```gdscript
# Minimum corridor length
walker.set_min_hall(3)  # default: 3

# Maximum corridor length
walker.set_max_hall(6)  # default: 6
```

##### Room Settings

```gdscript
# Room dimension parameter (affects room size)
walker.set_room_dim(5)  # default: 5
```

#### Generation

```gdscript
# Generate the dungeon
var result: WalkerResult = walker.generate()
```

### WalkerResult

Result object containing the generated dungeon data.

#### Basic Queries

```gdscript
# Get all generated floor positions (abstract generation output)
var floors: PackedVector2Array = result.get_floor_positions()

# Get all generated wall positions (positions adjacent to floors)
var walls: PackedVector2Array = result.get_wall_positions()

# Get map dimensions
var width: int = result.get_map_width()
var height: int = result.get_map_height()

# Get statistics
var stats: Dictionary = result.get_statistics()
# Returns: { floor_count, wall_count, map_width, map_height }
```

#### TileMapLayer Integration

**Important**: Use `get_tilemap_positions_with_atlas()` when you need positions that actually have specific tiles painted, such as spawning players or monsters.

```gdscript
# Query TileMapLayer for positions with specific atlas coordinates
var actual_floor_positions: PackedVector2Array = result.get_tilemap_positions_with_atlas(
    tilemap_layer,      # The TileMapLayer to query
    atlas_coords,       # Vector2i - atlas coords to match
    source_id           # int - tile source ID (default: 0)
)
```

**Why use this?**
- `get_floor_positions()` returns the abstract generation output - all positions that were marked as "floor" during generation
- `get_tilemap_positions_with_atlas()` returns only positions that actually have a specific tile painted on the TileMapLayer
- Use the latter when spawning entities to ensure they spawn on walkable tiles, not in walls or void areas

## Usage Examples

### Basic Generation

```gdscript
# Create generator
var walker = WalkerDungeonGenerator.new()
walker.set_allow_overlap(true)
walker.set_total_floor_count(500)

# Generate
var result = walker.generate()

# Use the result
var floors = result.get_floor_positions()
var walls = result.get_wall_positions()

print("Generated %d floor tiles" % floors.size())
```

### Paint to TileMapLayer

```gdscript
# Generate dungeon
var walker = WalkerDungeonGenerator.new()
walker.set_total_floor_count(400)
var result = walker.generate()

# Paint to TileMapLayer
var tilemap = $TileMapLayer
var floor_atlas = Vector2i(0, 0)
var wall_atlas = Vector2i(1, 0)

for pos in result.get_floor_positions():
    tilemap.set_cell(Vector2i(pos), 0, floor_atlas)

for pos in result.get_wall_positions():
    tilemap.set_cell(Vector2i(pos), 0, wall_atlas)
```

### Spawn Player on Floor Tile

```gdscript
# Generate and paint dungeon
var walker = WalkerDungeonGenerator.new()
walker.set_total_floor_count(300)
var result = walker.generate()

var tilemap = $TileMapLayer
var floor_atlas = Vector2i(0, 0)
var wall_atlas = Vector2i(1, 0)

# Paint tiles
for pos in result.get_floor_positions():
    tilemap.set_cell(Vector2i(pos), 0, floor_atlas)
for pos in result.get_wall_positions():
    tilemap.set_cell(Vector2i(pos), 0, wall_atlas)

# Spawn player on a random floor tile
var floor_positions = result.get_tilemap_positions_with_atlas(
    tilemap,
    floor_atlas,
    0  # source_id
)

if floor_positions.size() > 0:
    randomize()
    var spawn_index = randi_range(0, floor_positions.size() - 1)
    var spawn_pos = floor_positions[spawn_index]
    var world_pos = tilemap.map_to_local(Vector2i(spawn_pos))
    player.global_position = world_pos
```

### Spawn Multiple Monsters

```gdscript
# After generating and painting dungeon...

var floor_positions = result.get_tilemap_positions_with_atlas(
    tilemap,
    floor_atlas,
    0
)

# Spawn 10 monsters on random floor tiles
var monster_scene = preload("res://monster.tscn")
var num_monsters = 10

for i in range(num_monsters):
    if floor_positions.size() > 0:
        var spawn_index = randi_range(0, floor_positions.size() - 1)
        var spawn_pos = floor_positions[spawn_index]
        var world_pos = tilemap.map_to_local(Vector2i(spawn_pos))

        var monster = monster_scene.instantiate()
        monster.global_position = world_pos
        add_child(monster)

        # Remove position to avoid spawning multiple monsters on same tile
        floor_positions.remove_at(spawn_index)
```

### Reproducible Generation with Seeds

```gdscript
# Generate same dungeon every time
var walker = WalkerDungeonGenerator.new()
walker.set_use_seed(true)
walker.set_seed(42)
walker.set_total_floor_count(250)

var result = walker.generate()
# Will always generate the same dungeon
```

### Organic Cave Style

```gdscript
# Create organic, cave-like dungeons
var walker = WalkerDungeonGenerator.new()
walker.set_allow_overlap(true)      # Enable organic caves
walker.set_total_floor_count(800)   # Larger caves
walker.set_room_dim(7)              # Bigger rooms

var result = walker.generate()
```

### Structured Rooms Style

```gdscript
# Create dungeons with distinct rectangular rooms
var walker = WalkerDungeonGenerator.new()
walker.set_allow_overlap(false)     # Disable overlap
walker.set_total_floor_count(400)
walker.set_min_hall(4)              # Longer corridors
walker.set_max_hall(8)
walker.set_room_dim(6)              # Medium rooms

var result = walker.generate()
```

## Algorithm Details

### Overlap Mode (allow_overlap = true)

1. Spawns multiple walkers at the center or random positions
2. Each walker moves randomly in cardinal directions (N/S/E/W)
3. Walkers occasionally:
   - Change direction
   - Spawn new walkers
   - Place organic circular rooms
   - Widen corridors
4. Continues until `total_floor_count` is reached

**Result**: Organic, cave-like structures with flowing corridors and natural-looking rooms.

### Non-overlap Mode (allow_overlap = false)

1. Starts with a single rectangular room at the center
2. Repeatedly:
   - Picks a random starting point (existing room or floor tile)
   - Carves a corridor of random length in a random direction
   - Attempts to place a rectangular room at the end
   - Rooms have padding to prevent overlap
3. Continues until `total_floor_count` is reached

**Result**: Dungeon with distinct rectangular rooms connected by corridors, similar to traditional roguelikes.

### Map Size Calculation

The generator automatically calculates map size based on:
- `total_floor_count`
- `allow_overlap` mode
- Fill ratio (estimated density)

Formula:
- Overlap mode: More generous size to accommodate organic growth
- Non-overlap mode: Tighter size for structured rooms

## Performance Considerations

- **total_floor_count**: Higher values take longer to generate
  - 200-500: Fast (< 5ms)
  - 500-1000: Medium (5-20ms)
  - 1000+: Slower (20ms+)

- **allow_overlap**: Overlap mode is slightly faster as it doesn't check for room collisions

- **Map size**: Automatically calculated, but larger dungeons use more memory

## Tips & Best Practices

### For Cave-Like Dungeons
- Use `allow_overlap = true`
- Higher `total_floor_count` (600-1000)
- `room_dim = 6-8` for larger organic rooms

### For Roguelike Dungeons
- Use `allow_overlap = false`
- Moderate `total_floor_count` (300-500)
- `min_hall = 3-5`, `max_hall = 6-10`
- `room_dim = 4-7` for traditional room sizes

### Entity Spawning
- Always use `get_tilemap_positions_with_atlas()` for entity placement
- Cache the result if spawning many entities
- Remove positions from the array after spawning to avoid overlapping entities

### Seeds
- Use seeds for:
  - Debugging (reproducible bugs)
  - Procedural generation with save games
  - Testing specific layouts
- Different seeds with same parameters create different dungeons

## Common Issues

### Player Spawns Outside Dungeon

**Problem**: Using `get_floor_positions()` for spawning

**Solution**: Use `get_tilemap_positions_with_atlas()` instead:

```gdscript
# ❌ Wrong - returns abstract generation positions
var floors = result.get_floor_positions()

# ✅ Correct - returns actual painted floor tiles
var floors = result.get_tilemap_positions_with_atlas(
    tilemap,
    floor_atlas_coords,
    source_id
)
```

### Too Few Floor Tiles

**Problem**: `total_floor_count` is too low or generation hit max attempts

**Solution**: Increase `total_floor_count` or adjust room/corridor parameters

### Disconnected Regions

**Problem**: In non-overlap mode, rooms may occasionally be disconnected

**Solution**: Increase `total_floor_count` to add more connecting corridors

## See Also

- [BSPDungeonGenerator](BSPDungeonGenerator.md) - Binary space partitioning algorithm
- [DungeonPreview](DungeonPreview.md) - Visual debugging tool
- [OverlappingWFC](OverlappingWFC.md) - Wave Function Collapse generator
