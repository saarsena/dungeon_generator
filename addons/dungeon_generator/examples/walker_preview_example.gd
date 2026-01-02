@tool
extends Node2D

## Example script demonstrating Walker dungeon generation with DungeonPreview
##
## This script shows how to:
## 1. Generate dungeons using WalkerDungeonGenerator
## 2. Visualize results with DungeonPreview
## 3. Compare different algorithms side-by-side
## 4. Export results as PNG

# Generator settings
@export_group("Generator Settings")
@export var algorithm: String = "Walker"  # "Walker" or "WFC"
@export var allow_overlap: bool = true
@export var total_floor_count: int = 300
@export var min_hall: int = 3
@export var max_hall: int = 6
@export var room_dim: int = 5
@export var use_seed: bool = true
@export var seed_value: int = 12345

# Actions
@export_group("Actions")
@export var generate: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			generate_dungeon()
			generate = false

@export var generate_comparison: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			generate_comparison_view()
			generate_comparison = false

@export var export_png: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			export_preview()
			export_png = false

@export var clear_preview: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			clear()
			clear_preview = false

# ============================================================================
# GENERATION METHODS
# ============================================================================

func generate_dungeon() -> void:
	var preview = get_node_or_null("DungeonPreview")
	if not preview:
		push_error("No DungeonPreview node found! Add one to the scene.")
		return

	print("Generating %s dungeon..." % algorithm)
	var start_time = Time.get_ticks_msec()

	var result
	if algorithm == "Walker":
		result = generate_walker()
	elif algorithm == "WFC":
		result = generate_wfc()
	else:
		push_error("Unknown algorithm: %s" % algorithm)
		return

	var elapsed = Time.get_ticks_msec() - start_time
	print("Generation complete in %d ms" % elapsed)

	# Display result
	preview.display_result(result, 0)

func generate_walker() -> WalkerResult:
	var walker = WalkerDungeonGenerator.new()
	walker.set_allow_overlap(allow_overlap)
	walker.set_total_floor_count(total_floor_count)
	walker.set_min_hall(min_hall)
	walker.set_max_hall(max_hall)
	walker.set_room_dim(room_dim)
	walker.set_use_seed(use_seed)
	walker.set_seed(seed_value)

	return walker.generate()

func generate_wfc() -> WFCResult:
	var wfc = GDTilingWFCv2.new()
	wfc.use_connection_system()

	# Simple dungeon tileset
	wfc.add_connected_tile(0, {"borders_all": true}, 1.0)  # Floor
	wfc.add_connected_tile(1, {"left": true, "right": true}, 0.8)  # Horizontal corridor
	wfc.add_connected_tile(2, {"up": true, "down": true}, 0.8)  # Vertical corridor
	wfc.add_connected_tile(3, {"left": true, "up": true, "right": true, "down": true}, 0.5)  # Cross

	wfc.auto_generate_rules()
	wfc.set_size(20, 20)
	wfc.set_seed(seed_value if use_seed else randi())

	return wfc.run()

func generate_comparison_view() -> void:
	var preview = get_node_or_null("DungeonPreview")
	if not preview:
		push_error("No DungeonPreview node found!")
		return

	print("Generating comparison view...")

	# Enable split view
	preview.split_view_enabled = true

	# Generate Walker result (slot 0)
	var walker_result = generate_walker()
	preview.display_result(walker_result, 0)

	# Generate WFC result (slot 1)
	var wfc_result = generate_wfc()
	preview.display_result(wfc_result, 1)

	print("Comparison view ready: Walker (left) vs WFC (right)")

func export_preview() -> void:
	var preview = get_node_or_null("DungeonPreview")
	if not preview:
		push_error("No DungeonPreview node found!")
		return

	var path = "res://exports/dungeon_%s_%d.png" % [algorithm, Time.get_unix_time_from_system()]
	preview.export_as_png(path)
	print("Exported preview to: ", path)

func clear() -> void:
	var preview = get_node_or_null("DungeonPreview")
	if preview:
		preview.clear()
		print("Preview cleared")

# ============================================================================
# RUNTIME EXAMPLE
# ============================================================================

func _ready() -> void:
	if not Engine.is_editor_hint():
		# Example: Generate dungeon at runtime
		generate_dungeon()

		# Example: Populate a TileMap
		var tilemap = get_node_or_null("TileMap")
		if tilemap:
			populate_tilemap(tilemap)

func populate_tilemap(tilemap: TileMap) -> void:
	var preview = get_node_or_null("DungeonPreview")
	if not preview:
		return

	# Get floor and wall positions
	var floors = preview.floor_positions
	var walls = preview.wall_positions

	# Clear tilemap
	tilemap.clear()

	# Place floor tiles (tile_id = 0)
	for pos in floors:
		tilemap.set_cell(0, Vector2i(pos), 0, Vector2i(0, 0))

	# Place wall tiles (tile_id = 1)
	for pos in walls:
		tilemap.set_cell(0, Vector2i(pos), 1, Vector2i(0, 0))

	print("Populated TileMap with %d floors and %d walls" % [floors.size(), walls.size()])
