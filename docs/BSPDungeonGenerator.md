# BSPDungeonGenerator

**Inherits:** RefCounted

Binary Space Partitioning dungeon generator that creates rectangular rooms connected by corridors.

## Description

BSPDungeonGenerator uses the Binary Space Partitioning algorithm to create dungeon layouts with distinct rectangular rooms connected by L-shaped corridors. This algorithm is ideal for traditional roguelike dungeons with clear room boundaries.

The algorithm works by:
1. Recursively splitting the map into smaller partitions
2. Creating a room within each leaf partition
3. Connecting adjacent rooms with corridors

This is a pure C++ implementation for maximum performance.

## Properties

| Type | Property | Default |
|------|----------|---------|
| int | map_width | 64 |
| int | map_height | 64 |
| int | min_room_size | 5 |
| int | max_room_size | 12 |
| int | max_splits | 6 |
| int | room_padding | 1 |
| bool | use_seed | false |
| int | seed | 12345 |

## Methods

| Returns | Method |
|---------|--------|
| void | **set_map_size**(width: int, height: int) |
| void | **set_room_size_range**(min_size: int, max_size: int) |
| void | **set_max_splits**(splits: int) |
| void | **set_room_padding**(padding: int) |
| void | **set_use_seed**(enabled: bool) |
| void | **set_seed**(seed_value: int) |
| BSPResult | **generate**() |

## Property Descriptions

### map_width: int = 64
Width of the dungeon map in tiles. Larger values create bigger dungeons.

### map_height: int = 64
Height of the dungeon map in tiles. Larger values create bigger dungeons.

### min_room_size: int = 5
Minimum size for both width and height of generated rooms. Must be at least 3.

### max_room_size: int = 12
Maximum size for both width and height of generated rooms. Should be larger than min_room_size.

### max_splits: int = 6
Number of BSP tree splits to perform. Higher values create more, smaller rooms. Range: 1-8.

### room_padding: int = 1
Minimum distance between rooms and partition boundaries. Ensures rooms don't touch partition edges.

### use_seed: bool = false
Whether to use a fixed seed for deterministic generation. When true, the same seed will always produce the same dungeon.

### seed: int = 12345
Random seed value. Only used when use_seed is true.

## Method Descriptions

### set_map_size(width: int, height: int) -> void
Set the overall dungeon dimensions in tiles.

```gdscript
var bsp = BSPDungeonGenerator.new()
bsp.set_map_size(100, 80)
```

### set_room_size_range(min_size: int, max_size: int) -> void
Set the minimum and maximum dimensions for generated rooms.

```gdscript
bsp.set_room_size_range(6, 15)  # Rooms will be 6x6 to 15x15
```

### set_max_splits(splits: int) -> void
Set the number of BSP tree splits. More splits = more rooms.

```gdscript
bsp.set_max_splits(7)  # Create many smaller rooms
bsp.set_max_splits(3)  # Create fewer larger rooms
```

### set_room_padding(padding: int) -> void
Set the minimum gap between rooms and partition edges.

```gdscript
bsp.set_room_padding(2)  # Rooms will be at least 2 tiles from edges
```

### set_use_seed(enabled: bool) -> void
Enable or disable deterministic generation with a fixed seed.

```gdscript
bsp.set_use_seed(true)
bsp.set_seed(42)  # Always generates the same dungeon
```

### set_seed(seed_value: int) -> void
Set the random seed value. Only affects generation when use_seed is true.

### generate() -> BSPResult
Generate a new dungeon and return the result containing floor, corridor, and wall positions.

```gdscript
var result = bsp.generate()
var floors = result.get_floor_positions()
var corridors = result.get_corridor_positions()
var walls = result.get_wall_positions()
```

## Usage Example

### Basic Usage

```gdscript
# Create generator
var bsp = BSPDungeonGenerator.new()

# Configure
bsp.set_map_size(80, 60)
bsp.set_room_size_range(6, 12)
bsp.set_max_splits(6)

# Generate
var result = bsp.generate()

# Use the result
for pos in result.get_floor_positions():
    tilemap.set_cell(Vector2i(pos), floor_tile_id, atlas_coords)
```

### Deterministic Generation

```gdscript
# Same seed always produces same dungeon
var bsp = BSPDungeonGenerator.new()
bsp.set_use_seed(true)
bsp.set_seed(12345)
var result = bsp.generate()  # Identical every time
```

### Room Variety

```gdscript
# Fewer large rooms
bsp.set_max_splits(3)
bsp.set_room_size_range(8, 20)

# Many small rooms
bsp.set_max_splits(8)
bsp.set_room_size_range(4, 8)
```

## Performance

BSPDungeonGenerator is implemented in C++ for maximum performance:

- Small dungeon (64x64): ~1-2ms
- Medium dungeon (128x128): ~5-10ms
- Large dungeon (256x256): ~20-40ms

Performance scales roughly linearly with map area.

## Algorithm Details

The BSP algorithm:

1. **Partitioning**: Recursively divides the map into smaller rectangles
   - Each split is either horizontal or vertical (chosen randomly)
   - Splits continue until max_splits is reached or rectangles are too small

2. **Room Creation**: Places a random-sized room within each leaf partition
   - Room size is between min_room_size and max_room_size
   - Room position is randomized within the partition
   - room_padding ensures rooms don't touch partition edges

3. **Corridor Connection**: Connects adjacent rooms with L-shaped corridors
   - Picks representative points from each room (usually centers)
   - Creates corridors that move horizontally then vertically (or vice versa)
   - Order is randomized for variety
   - **Corridors have floor tiles**: Both `get_floor_positions()` and `get_corridor_positions()` include corridor tiles

4. **Wall Generation**: Automatically places walls around all floor and corridor tiles

**Note:** In the BSPResult, corridor tiles are included in both `get_floor_positions()` (for ease of tilemap placement) and `get_corridor_positions()` (if you need to distinguish them). This means you can treat all walkable tiles uniformly as "floors".

## See Also

- [BSPResult](BSPResult.md) - Result object containing generated dungeon data
- [WalkerDungeonGenerator](WalkerDungeonGenerator.md) - For organic cave-like dungeons
- [GDTilingWFCv2](GDTilingWFCv2.md) - For constraint-based generation
