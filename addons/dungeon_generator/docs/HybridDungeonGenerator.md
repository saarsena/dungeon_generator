# HybridDungeonGenerator

**Inherits:** RefCounted

A multi-phase procedural dungeon generator that combines physics-based room placement, graph theory (Delaunay/MST), random walkers, and cellular automata.

## Description

HybridDungeonGenerator provides a sophisticated, "best of all worlds" approach to dungeon generation. It produces varied layouts that feel both structured (via rooms and corridors) and organic (via walker-carved paths and CA-smoothed walls).

The generation process occurs in several internal phases:
1. **Physics Phase**: Rooms are spawned in a cluster and pushed apart using a simple steering behavior to prevent overlaps.
2. **Graph Phase**: A Delaunay triangulation is computed between "Main Rooms", followed by a Minimum Spanning Tree (MST) to ensure connectivity. Some extra edges are added back for non-linear layouts.
3. **Raster Phase**: The rooms and graph connections are drawn onto an internal high-resolution grid.
4. **Walker Phase**: Multiple agents are spawned to carve organic paths, connecting the structured elements with more natural, cave-like passages.
5. **Automata Phase**: Cellular Automata (CA) passes smooth the overall shape, followed by a connectivity prune to ensure the entire dungeon is reachable.

## Properties

| Type | Property | Default |
|------|----------|---------|
| int | room_count | 150 |
| float | spread_radius | 50.0 |
| int | walker_count | 400 |
| int | grid_width | 200 |
| int | grid_height | 150 |
| int | tile_w | 4 |
| int | tile_h | 4 |
| int | seed | 0 |

## Methods

| Returns | Method |
|---------|--------|
| void | **set_room_count**(count: int) |
| void | **set_spread_radius**(radius: float) |
| void | **set_walker_count**(count: int) |
| void | **set_grid_size**(width: int, height: int) |
| void | **set_tile_size**(w: int, h: int) |
| void | **set_seed**(seed_value: int) |
| HybridResult | **generate**() |

## Property Descriptions

### room_count: int = 150
Total number of rooms initially spawned. Note that only larger rooms are typically designated as "Main Rooms" for graph connectivity.

### spread_radius: float = 50.0
The radius of the initial circle where rooms are spawned. Larger values result in more spread-out dungeons.

### walker_count: int = 400
The number of random walkers used to carve organic paths between rooms.

### grid_width: int = 200
Internal grid resolution width.

### grid_height: int = 150
Internal grid resolution height.

### tile_w: int = 4
The width scale. The internal world width is `grid_width * tile_w`.

### tile_h: int = 4
The height scale. The internal world height is `grid_height * tile_h`.

### seed: int = 0
Random seed for generation. Use `0` for a random seed on every run.

## Method Descriptions

### set_grid_size(width: int, height: int) -> void
Convenience method to set both `grid_width` and `grid_height`.

### set_tile_size(w: int, h: int) -> void
Convenience method to set both `tile_w` and `tile_h`.

### generate() -> HybridResult
Executes the full multi-phase generation and returns a result object.

```gdscript
var result = hybrid.generate()
if result:
    var floors = result.get_floors()
    var walls = result.get_walls()
```

## Internal Grid vs. TileMap Coordinates

The generator works on an internal grid of size `grid_width` x `grid_height`. 
- `get_floors()` and `get_walls()` return positions in **grid coordinates** (e.g., from `(0,0)` to `(199, 149)`).
- These map directly to Godot's `TileMapLayer` cell coordinates.

The `tile_w` and `tile_h` properties define the scale of the "Physics World" relative to this grid. If `tile_w = 4`, then a room with width `20` in world units will occupy `5` tiles in the final grid.

## Usage Example

### Basic Generation

```gdscript
var hybrid = HybridDungeonGenerator.new()

# Configure
hybrid.room_count = 200
hybrid.walker_count = 500
hybrid.set_grid_size(250, 200)

# Generate
var result = hybrid.generate()

# Place in TileMap
for pos in result.get_floors():
    tilemap.set_cell(Vector2i(pos), 0, Vector2i(0, 0)) # Floor
for pos in result.get_walls():
    tilemap.set_cell(Vector2i(pos), 0, Vector2i(1, 0)) # Wall
```

### Accessing Room Data

The `HybridResult` also allows you to access the room metadata, which is useful for spawning enemies or items.

```gdscript
var rooms = result.get_rooms()
for room in rooms:
    if room.is_main:
        print("Main Room at: ", room.x, ", ", room.y)
        # Convert world x/y to grid coordinates for spawning
        var grid_x = int(room.x / result.get_tile_w())
        var grid_y = int(room.y / result.get_tile_h())
```

## Performance

The Hybrid generator is highly optimized C++:
- Standard size (200x150): ~15-30ms
- Large size (500x500): ~100-200ms

Performance depends heavily on `room_count` (affects Physics phase) and grid area (affects Raster/Walker/CA phases).

## See Also

- [HybridResult](HybridResult.md) - Result object containing detailed dungeon data
- [BSPDungeonGenerator](BSPDungeonGenerator.md) - For strictly rectangular dungeons
- [WalkerDungeonGenerator](WalkerDungeonGenerator.md) - For pure organic caves
