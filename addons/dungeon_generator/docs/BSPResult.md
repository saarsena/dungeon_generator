# BSPResult

**Inherits:** RefCounted

Result object containing the output of BSP dungeon generation.

## Description

BSPResult holds the generated dungeon data from BSPDungeonGenerator. It contains three types of positions:

- **Floor positions**: Tiles inside rooms
- **Corridor positions**: Tiles in hallways connecting rooms
- **Wall positions**: Border tiles surrounding rooms and corridors

All positions are stored as PackedVector2Array for efficient memory usage and fast iteration.

## Methods

| Returns | Method |
|---------|--------|
| PackedVector2Array | **get_floor_positions**() |
| PackedVector2Array | **get_corridor_positions**() |
| PackedVector2Array | **get_wall_positions**() |
| int | **get_floor_count**() |

## Method Descriptions

### get_floor_positions() -> PackedVector2Array
Returns all floor tile positions inside rooms (not including corridors).

```gdscript
var result = bsp.generate()
for pos in result.get_floor_positions():
    print("Room floor at: ", pos)
```

### get_corridor_positions() -> PackedVector2Array
Returns all corridor tile positions connecting rooms.

Corridors are L-shaped paths between room centers. You can render them differently from floor tiles for visual variety.

```gdscript
var corridors = result.get_corridor_positions()
for pos in corridors:
    tilemap.set_cell(Vector2i(pos), corridor_tile_id, atlas_coords)
```

### get_wall_positions() -> PackedVector2Array
Returns all wall tile positions surrounding the dungeon.

Walls are automatically generated around all floor and corridor tiles, forming the dungeon boundaries.

```gdscript
var walls = result.get_wall_positions()
for pos in walls:
    tilemap.set_cell(Vector2i(pos), wall_tile_id, atlas_coords)
```

### get_floor_count() -> int
Returns the total number of floor tiles (excluding corridors).

Useful for statistics and difficulty scaling.

```gdscript
var floor_count = result.get_floor_count()
print("Generated ", floor_count, " floor tiles")
```

## Usage Example

### Complete Dungeon Population

```gdscript
func populate_dungeon(result: BSPResult, tilemap: TileMapLayer):
    # Clear existing tiles
    tilemap.clear()

    # Place room floors
    for pos in result.get_floor_positions():
        tilemap.set_cell(Vector2i(pos), 0, Vector2i(0, 0))

    # Place corridors (optionally different tile)
    for pos in result.get_corridor_positions():
        tilemap.set_cell(Vector2i(pos), 0, Vector2i(1, 0))

    # Place walls
    for pos in result.get_wall_positions():
        tilemap.set_cell(Vector2i(pos), 0, Vector2i(2, 0))
```

### Get All Walkable Tiles

```gdscript
func get_walkable_positions(result: BSPResult) -> PackedVector2Array:
    var walkable = PackedVector2Array()
    walkable.append_array(result.get_floor_positions())
    walkable.append_array(result.get_corridor_positions())
    return walkable
```

### Statistics

```gdscript
var result = bsp.generate()
print("Floor tiles: ", result.get_floor_count())
print("Corridor tiles: ", result.get_corridor_positions().size())
print("Wall tiles: ", result.get_wall_positions().size())
print("Total walkable: ", result.get_floor_count() + result.get_corridor_positions().size())
```

## Notes

- All positions use integer coordinates (whole tiles)
- Positions are in tilemap space, not pixel space
- Corridors are kept separate from floors so you can style them differently
- Walls include all 8 surrounding directions (including diagonals)

## See Also

- [BSPDungeonGenerator](BSPDungeonGenerator.md) - The generator that produces this result
- [WalkerResult](WalkerResult.md) - Similar result object for Walker algorithm
- [WFCResult](WFCResult.md) - Similar result object for WFC algorithm
