@tool
extends Node2D
class_name DungeonPreview

## DungeonPreview - Visual debug helper for procedural dungeon generators
##
## Optional development tool that visualizes dungeon generation results.
## Supports WalkerResult, WFCResult, and future generator results.
##
## Features:
## - Live preview in editor
## - Statistics overlay
## - Split-view comparison
## - PNG export
## - Grid overlay with coordinates
## - LOD for large dungeons
## - Auto-cleanup with remove_self_on_ready

# ============================================================================
# EXPORTS - Inspector Properties
# ============================================================================

## Algorithm Selection
enum Algorithm { WFC, WALKER, BSP, OVERLAPPING_WFC }

@export_group("Generator")
@export var algorithm: Algorithm = Algorithm.WFC:
	set(value):
		algorithm = value
		notify_property_list_changed()

@export var generate: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			generate_dungeon()
		generate = false

## Walker Parameters
@export_group("Walker Settings")
@export var walker_allow_overlap: bool = true
@export var walker_total_floor_count: int = 300
@export_range(1, 20) var walker_min_hall: int = 3
@export_range(1, 30) var walker_max_hall: int = 6
@export_range(2, 15) var walker_room_dim: int = 5
@export var walker_use_seed: bool = false
@export var walker_seed: int = 12345

## WFC Parameters
@export_group("WFC Settings")
@export var wfc_width: int = 20
@export var wfc_height: int = 20
@export var wfc_use_seed: bool = false
@export var wfc_seed: int = 12345
@export var wfc_use_stamps: bool = true
@export_subgroup("Tile Weights (0-15)")
@export var wfc_tile_weights: Dictionary = {
	0: 1.0, 1: 1.0, 2: 1.0, 3: 1.0,
	4: 1.0, 5: 1.0, 6: 1.0, 7: 1.0,
	8: 1.0, 9: 1.0, 10: 1.0, 11: 1.0,
	12: 1.0, 13: 1.0, 14: 1.0, 15: 1.0
}

## BSP Parameters
@export_group("BSP Settings")
@export var bsp_map_width: int = 80
@export var bsp_map_height: int = 60
@export_range(2, 10) var bsp_min_room_size: int = 4
@export_range(5, 20) var bsp_max_room_size: int = 12
@export_range(1, 5) var bsp_max_depth: int = 4
@export var bsp_use_seed: bool = false
@export var bsp_seed: int = 12345

## Overlapping WFC Parameters
@export_group("Overlapping WFC Settings")
@export_file("*.png") var owfc_seed_image_path: String = "":
	set(value):
		owfc_seed_image_path = value
		if Engine.is_editor_hint() and algorithm == Algorithm.OVERLAPPING_WFC:
			_load_owfc_seed_image()
			queue_redraw()

@export_range(8, 512, 1) var owfc_output_width: int = 20
@export_range(8, 512, 1) var owfc_output_height: int = 20
@export_range(2, 5, 1) var owfc_pattern_size: int = 2
@export_range(1, 8, 1) var owfc_symmetry: int = 8
@export var owfc_use_seed: bool = false
@export var owfc_seed: int = 12345
@export var owfc_use_stamps: bool = false
@export_range(1, 5, 1) var owfc_stamp_size: int = 3
@export var owfc_use_default_mapping: bool = false
@export var owfc_show_seed_preview: bool = true:
	set(value):
		owfc_show_seed_preview = value
		queue_redraw()

@export_subgroup("Custom Mappings")
@export var owfc_custom_pattern_mappings: Dictionary = {}  # color_value -> tile_id
@export var owfc_custom_tile_stamps: Dictionary = {}  # tile_id -> {pattern: PackedInt32Array, width: int, height: int}
@export_subgroup("Advanced Options")
@export var owfc_periodic_input: bool = true
@export var owfc_periodic_output: bool = false
@export var owfc_ground_mode: bool = false  ## WARNING: May crash with small seed images! Only use for large, periodic images with consistent bottom rows.

## Display Settings
@export_group("Display Settings")
@export var tile_size: Vector2 = Vector2(16, 16):
	set(value):
		tile_size = Vector2(max(4, value.x), max(4, value.y))
		queue_redraw()

@export var floor_color: Color = Color(0.4, 0.4, 0.5, 1.0):
	set(value):
		floor_color = value
		queue_redraw()

@export var wall_color: Color = Color(0.2, 0.15, 0.3, 1.0):
	set(value):
		wall_color = value
		queue_redraw()

@export var background_color: Color = Color(0.1, 0.1, 0.1, 1.0):
	set(value):
		background_color = value
		queue_redraw()

@export var center_view: bool = true:
	set(value):
		center_view = value
		_update_view_centering()

## Grid Overlay
@export_group("Grid Overlay")
@export var show_grid: bool = false:
	set(value):
		show_grid = value
		queue_redraw()

@export var grid_color: Color = Color(0.3, 0.3, 0.3, 0.3):
	set(value):
		grid_color = value
		queue_redraw()

@export var show_coordinates: bool = false:
	set(value):
		show_coordinates = value
		queue_redraw()

@export var coordinate_interval: int = 10:
	set(value):
		coordinate_interval = max(1, value)
		queue_redraw()

## Statistics Overlay
@export_group("Statistics")
@export var show_statistics: bool = true:
	set(value):
		show_statistics = value
		queue_redraw()

@export var stats_position: Vector2 = Vector2(10, 10):
	set(value):
		stats_position = value
		queue_redraw()

@export var stats_background: bool = true
@export var stats_font_size: int = 14

## Comparison Mode
@export_group("Comparison")
@export var split_view_enabled: bool = false:
	set(value):
		split_view_enabled = value
		queue_redraw()

@export_enum("Horizontal", "Vertical") var split_direction: int = 0:
	set(value):
		split_direction = value
		queue_redraw()

## Performance
@export_group("Performance")
@export var enable_lod: bool = true
@export var lod_threshold: int = 2000:
	set(value):
		lod_threshold = max(100, value)

@export var max_visible_tiles: int = 10000:
	set(value):
		max_visible_tiles = max(100, value)

## Export
@export_group("Export")
@export var export_path: String = "res://dungeon_preview.png"
@export var export_scale: float = 1.0

## TileMapLayer Integration
@export_group("TileMapLayer")
@export var target_tilemap_layer: TileMapLayer = null
@export var auto_populate_tilemap: bool = false
@export var populate_tilemap_button: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			populate_tilemap()
		populate_tilemap_button = false

@export_subgroup("Tile Mapping (Floor/Wall Mode)")
@export var floor_tile_source_id: int = 0
@export var floor_tile_atlas_coords: Vector2i = Vector2i(0, 0)
@export var wall_tile_source_id: int = 0
@export var wall_tile_atlas_coords: Vector2i = Vector2i(1, 0)

@export_subgroup("Tile Mapping (Multi-Color Mode)")
@export var multi_color_tile_mappings: Dictionary = {}  ## Maps tile_value (int) -> Vector2i(atlas_x, atlas_y)
@export var multi_color_tile_source_id: int = 0
@export var auto_generate_mappings_button: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			_auto_generate_tile_mappings()
		auto_generate_mappings_button = false
@export_enum("Horizontal:0", "Vertical:1") var auto_mapping_direction: int = 0  ## Direction to assign tiles in atlas
@export var auto_mapping_start_coords: Vector2i = Vector2i(0, 0)  ## Starting atlas coordinates for auto-mapping

## Safety
@export_group("Safety")
@export var remove_self_on_ready: bool = false

# ============================================================================
# WFC TILE DEFINITIONS
# ============================================================================

# Tile definitions: [left, up, right, down]
# true = connection exists, false = no connection
const TILE_DEFINITIONS = {
	# Terminal nodes (1 connection)
	0: {"left": true},          # Left only
	1: {"up": true},            # Up only
	2: {"right": true},         # Right only
	3: {"down": true},          # Down only

	# L-shaped tiles (2 connections)
	4: {"left": true, "up": true},         # Left-Up
	5: {"left": true, "right": true},      # Left-Right (horizontal corridor)
	6: {"left": true, "down": true},       # Left-Down
	7: {"up": true, "right": true},        # Up-Right
	8: {"up": true, "down": true},         # Up-Down (vertical corridor)
	9: {"right": true, "down": true},      # Right-Down

	# T-shaped tiles (3 connections)
	10: {"left": true, "up": true, "right": true},       # Left-Up-Right
	11: {"left": true, "up": true, "down": true},        # Left-Up-Down
	12: {"left": true, "right": true, "down": true},     # Left-Right-Down
	13: {"up": true, "right": true, "down": true},       # Up-Right-Down

	# Cross (4 connections)
	14: {"left": true, "up": true, "right": true, "down": true},  # All directions

	# Empty space (borders all)
	15: {"borders_all": true}
}

# 3x3 stamp patterns for each WFC tile (0=floor, 1=wall)
const STAMP_PATTERNS = {
	0: [1,1,1, 0,0,1, 1,1,1],   # Left only
	1: [1,0,1, 1,0,1, 1,1,1],   # Up only
	2: [1,1,1, 1,0,0, 1,1,1],   # Right only
	3: [1,1,1, 1,0,1, 1,0,1],   # Down only
	4: [1,0,1, 0,0,1, 1,1,1],   # Left-Up corner
	5: [1,1,1, 0,0,0, 1,1,1],   # Left-Right corridor
	6: [1,1,1, 0,0,1, 1,0,1],   # Left-Down corner
	7: [1,0,1, 1,0,0, 1,1,1],   # Up-Right corner
	8: [1,0,1, 1,0,1, 1,0,1],   # Up-Down corridor
	9: [1,1,1, 1,0,0, 1,0,1],   # Right-Down corner
	10: [1,0,1, 0,0,0, 1,1,1],  # Left-Up-Right T
	11: [1,0,1, 0,0,1, 1,0,1],  # Left-Up-Down T
	12: [1,1,1, 0,0,0, 1,0,1],  # Left-Right-Down T
	13: [1,0,1, 1,0,0, 1,0,1],  # Up-Right-Down T
	14: [1,0,1, 0,0,0, 1,0,1],  # Cross (all 4)
	15: [0,0,0, 0,0,0, 0,0,0]   # Empty/floor (inverted)
}

# ============================================================================
# INTERNAL STATE
# ============================================================================

var floor_positions: PackedVector2Array = PackedVector2Array()
var wall_positions: PackedVector2Array = PackedVector2Array()

# For split view comparison
var floor_positions_b: PackedVector2Array = PackedVector2Array()
var wall_positions_b: PackedVector2Array = PackedVector2Array()

# For multi-color tile visualization (Overlapping WFC)
var multi_color_tiles: Dictionary = {}  # Vector2 -> tile_id
var using_multi_color_mode: bool = false

# Statistics
var stats: Dictionary = {}
var generation_time_ms: float = 0.0

# Bounds for centering and culling
var bounds_min: Vector2 = Vector2.ZERO
var bounds_max: Vector2 = Vector2.ZERO

# Overlapping WFC seed image
var owfc_seed_image: Image = null
var owfc_seed_preview_texture: Texture2D = null
var owfc_extracted_colors: Array[Color] = []  # Unique colors from seed image
var owfc_color_palette: Dictionary = {}  # color_value (int) -> Color mapping from seed image

# ============================================================================
# LIFECYCLE
# ============================================================================

func _ready() -> void:
	if remove_self_on_ready and not Engine.is_editor_hint():
		print("DungeonPreview: Removing self from tree (safety cleanup)")
		queue_free()

# ============================================================================
# GENERATION
# ============================================================================

func generate_dungeon() -> void:
	print("DungeonPreview: Generating %s dungeon..." % Algorithm.keys()[algorithm])
	var start_time = Time.get_ticks_msec()

	var result
	match algorithm:
		Algorithm.WALKER:
			result = _generate_walker()
		Algorithm.WFC:
			result = _generate_wfc()
		Algorithm.BSP:
			result = _generate_bsp()
		Algorithm.OVERLAPPING_WFC:
			result = _generate_overlapping_wfc()

	if result:
		var elapsed = Time.get_ticks_msec() - start_time
		print("DungeonPreview: Generation complete in %d ms" % elapsed)
		display_result(result, 0)

		# Populate TileMapLayer if configured
		if auto_populate_tilemap and target_tilemap_layer:
			populate_tilemap()
	else:
		push_error("DungeonPreview: Generation failed")

func _generate_walker() -> WalkerResult:
	var walker = WalkerDungeonGenerator.new()
	walker.set_allow_overlap(walker_allow_overlap)
	walker.set_total_floor_count(walker_total_floor_count)
	walker.set_min_hall(walker_min_hall)
	walker.set_max_hall(walker_max_hall)
	walker.set_room_dim(walker_room_dim)
	walker.set_use_seed(walker_use_seed)
	walker.set_seed(walker_seed)
	return walker.generate()

func _generate_wfc() -> WFCResult:
	var wfc = GDTilingWFCv2.new()
	wfc.use_connection_system()

	# Set stamp size if stamps are enabled
	if wfc_use_stamps:
		wfc.get_configuration().set_stamp_size(3)

	# Add all 16 connection-based tiles
	for tile_id in range(16):
		var connections = TILE_DEFINITIONS[tile_id]
		var weight = wfc_tile_weights.get(tile_id, 1.0)
		wfc.add_connected_tile(tile_id, connections, weight)

		# Add stamp pattern if enabled
		if wfc_use_stamps and STAMP_PATTERNS.has(tile_id):
			var stamp_pattern = PackedInt32Array(STAMP_PATTERNS[tile_id])
			wfc.set_tile_stamp(tile_id, stamp_pattern, 3, 3)

	# Auto-generate neighbor rules from connections
	wfc.auto_generate_rules()

	# Set generation parameters
	wfc.set_size(wfc_width, wfc_height)
	wfc.set_seed(wfc_seed if wfc_use_seed else randi())

	return wfc.run()

func _generate_bsp() -> BSPResult:
	var bsp = BSPDungeonGenerator.new()
	bsp.set_map_size(bsp_map_width, bsp_map_height)
	bsp.set_room_size_range(bsp_min_room_size, bsp_max_room_size)
	bsp.set_max_splits(bsp_max_depth)
	bsp.set_use_seed(bsp_use_seed)
	bsp.set_seed(bsp_seed)
	return bsp.generate()

func _generate_overlapping_wfc() -> OverlappingWFCResult:
	# Load seed image if not already loaded
	if not owfc_seed_image:
		_load_owfc_seed_image()

	if not owfc_seed_image:
		push_error("DungeonPreview: No seed image loaded for Overlapping WFC")
		return null

	var generator = OverlappingWFCGenerator.new()
	generator.enable_debug(true)  # Enable C++ debug output
	generator.set_seed_image(owfc_seed_image)
	generator.set_output_size(owfc_output_width, owfc_output_height)
	generator.set_pattern_size(owfc_pattern_size)
	generator.set_symmetry(owfc_symmetry)
	generator.set_use_seed(owfc_use_seed)
	generator.set_seed(owfc_seed)
	generator.set_periodic_input(owfc_periodic_input)
	generator.set_periodic_output(owfc_periodic_output)
	generator.set_ground_mode(owfc_ground_mode)

	# Configure stamps
	generator.enable_stamps(owfc_use_stamps)
	generator.set_stamp_size(owfc_stamp_size)

	# Set up default dungeon mapping if enabled
	if owfc_use_default_mapping:
		generator.setup_default_dungeon_mapping()
		generator.setup_default_dungeon_stamps()

	# Apply custom pattern mappings
	for color_value in owfc_custom_pattern_mappings:
		generator.add_pattern_to_tile_mapping(int(color_value), owfc_custom_pattern_mappings[color_value])

	# Apply custom tile stamps
	for tile_id in owfc_custom_tile_stamps:
		var stamp_data = owfc_custom_tile_stamps[tile_id]
		if stamp_data.has("pattern") and stamp_data.has("width") and stamp_data.has("height"):
			generator.set_tile_stamp(tile_id, stamp_data["pattern"], stamp_data["width"], stamp_data["height"])

	var result = generator.generate()

	if result and not result.is_success():
		var reason = result.get_failure_reason()
		print("DungeonPreview: Overlapping WFC failed: ", reason)

		if "contradiction" in reason.to_lower():
			print("  ⚠ WFC couldn't find a valid solution. Try:")
			print("    • Reduce output size (seed: %dx%d, output: %dx%d)" % [
				owfc_seed_image.get_width(),
				owfc_seed_image.get_height(),
				owfc_output_width,
				owfc_output_height
			])
			print("    • Decrease pattern_size (current: %d) → smaller patterns = more flexible" % owfc_pattern_size)
			print("    • Increase symmetry (current: %d) → more pattern variations" % owfc_symmetry)
			print("    • Disable ground_mode (crashes with small images)")
			print("    • For tiling: enable periodic_input and periodic_output")

	return result

# ============================================================================
# DRAWING
# ============================================================================

func _draw() -> void:
	# Draw seed image preview for Overlapping WFC (in editor only)
	if Engine.is_editor_hint() and algorithm == Algorithm.OVERLAPPING_WFC and owfc_show_seed_preview and owfc_seed_preview_texture:
		_draw_seed_preview()

	if floor_positions.is_empty() and wall_positions.is_empty():
		_draw_empty_state()
		return

	# Draw background
	if background_color.a > 0:
		var bg_rect = _get_visible_rect()
		draw_rect(bg_rect, background_color)

	# Draw grid if enabled
	if show_grid:
		_draw_grid()

	# Draw dungeon tiles
	if split_view_enabled:
		_draw_split_view()
	else:
		_draw_single_view()

	# Draw statistics overlay
	if show_statistics and not stats.is_empty():
		_draw_statistics()

	# Draw coordinates
	if show_coordinates:
		_draw_coordinates()

# ============================================================================
# PUBLIC API - Display Results
# ============================================================================

## Display a dungeon generation result (WalkerResult, WFCResult, etc.)
func display_result(result: RefCounted, slot: int = 0) -> void:
	if result == null:
		push_error("DungeonPreview: Cannot display null result")
		return

	var start_time = Time.get_ticks_msec()

	# Extract floor and wall positions based on result type
	var floors: PackedVector2Array
	var walls: PackedVector2Array

	# Special handling for OverlappingWFCResult with multi-color images
	if result.has_method("get_pattern_output") and algorithm == Algorithm.OVERLAPPING_WFC:
		if owfc_use_default_mapping:
			# Use standard floor/wall extraction (black→floor, white→wall)
			using_multi_color_mode = false
			multi_color_tiles.clear()
			if result.has_method("get_floor_positions"):
				floors = result.get_floor_positions(0)
			if result.has_method("get_wall_positions"):
				walls = result.get_wall_positions(1)

			# Warn if using default mapping with multi-color image
			if owfc_extracted_colors.size() > 2:
				print("DungeonPreview: NOTE - Seed image has %d colors, but 'Use Default Mapping' is enabled." % owfc_extracted_colors.size())
				print("  This only maps black→floor and white→wall. For multi-color visualization, uncheck 'Owfc Use Default Mapping'.")
		else:
			# Multi-color mode: store tile values with positions
			using_multi_color_mode = true
			multi_color_tiles.clear()

			# Debug: Check all output arrays
			print("DungeonPreview: Checking result outputs...")
			print("  - pattern_output size: %d, dimensions: %dx%d" % [
				result.get_pattern_output().size(),
				result.get_output_width(),
				result.get_output_height()
			])
			print("  - expanded_output size: %d, dimensions: %dx%d" % [
				result.get_expanded_output().size(),
				result.get_expanded_width(),
				result.get_expanded_height()
			])

			var output = result.get_pattern_output()  # Always use pattern output for multi-color mode
			var width = result.get_output_width()
			var height = result.get_output_height()

			print("DungeonPreview: Multi-color mode - output size: %d, dimensions: %dx%d" % [output.size(), width, height])
			print("DungeonPreview: Color palette size: %d" % owfc_color_palette.size())

			floors = PackedVector2Array()
			walls = PackedVector2Array()

			# Track unique tile values for debugging
			var unique_tiles: Dictionary = {}

			for y in range(height):
				for x in range(width):
					var idx = y * width + x
					if idx < output.size():
						var tile_value = output[idx]
						var pos = Vector2(x, y)

						# Store tile value for color lookup
						multi_color_tiles[pos] = tile_value

						# Track unique values
						unique_tiles[tile_value] = true

						# Also add to floors for bounds calculation (all tiles are "floors" for layout purposes)
						floors.append(pos)

			print("DungeonPreview: Found %d unique tile values in output" % unique_tiles.size())
			print("DungeonPreview: Added %d floor positions" % floors.size())
	else:
		# Try different result types
		if result.has_method("get_floor_positions"):
			floors = result.get_floor_positions()
		if result.has_method("get_wall_positions"):
			walls = result.get_wall_positions()

	# Get statistics if available
	if result.has_method("get_statistics"):
		stats = result.get_statistics()
	elif result.has_method("get_tile_distribution"):
		stats = result.get_tile_distribution()

	generation_time_ms = Time.get_ticks_msec() - start_time

	if slot == 0:
		set_floor_positions(floors)
		set_wall_positions(walls)
	else:
		set_floor_positions_b(floors)
		set_wall_positions_b(walls)

	print("DungeonPreview: Displayed result with %d floors, %d walls (slot %d)" % [
		floors.size(), walls.size(), slot
	])

## Set floor positions directly
func set_floor_positions(positions: PackedVector2Array) -> void:
	floor_positions = positions
	_update_bounds()
	_update_view_centering()
	queue_redraw()

## Set wall positions directly
func set_wall_positions(positions: PackedVector2Array) -> void:
	wall_positions = positions
	_update_bounds()
	queue_redraw()

## Set floor positions for split view comparison (slot B)
func set_floor_positions_b(positions: PackedVector2Array) -> void:
	floor_positions_b = positions
	queue_redraw()

## Set wall positions for split view comparison (slot B)
func set_wall_positions_b(positions: PackedVector2Array) -> void:
	wall_positions_b = positions
	queue_redraw()

## Clear all displayed data
func clear() -> void:
	floor_positions.clear()
	wall_positions.clear()
	floor_positions_b.clear()
	wall_positions_b.clear()
	multi_color_tiles.clear()
	using_multi_color_mode = false
	stats.clear()
	generation_time_ms = 0.0
	queue_redraw()

## Populate a TileMapLayer with the generated dungeon
func populate_tilemap() -> void:
	if not target_tilemap_layer:
		push_error("DungeonPreview: No target TileMapLayer assigned")
		return

	if floor_positions.is_empty():
		push_warning("DungeonPreview: No dungeon data to populate TileMapLayer")
		return

	# Debug: Check TileMapLayer state
	print("DungeonPreview: TileMapLayer debug info:")
	print("  - Node: ", target_tilemap_layer.name)
	print("  - Visible: ", target_tilemap_layer.visible)
	print("  - TileSet: ", target_tilemap_layer.tile_set)
	if target_tilemap_layer.tile_set:
		print("  - TileSet sources: ", target_tilemap_layer.tile_set.get_source_count())

	# Clear the tilemap layer
	target_tilemap_layer.clear()

	# Multi-color mode (Overlapping WFC)
	if using_multi_color_mode and not multi_color_tiles.is_empty():
		print("DungeonPreview: Populating TileMapLayer with %d multi-color tiles..." % multi_color_tiles.size())

		var tiles_placed = 0
		var unmapped_tiles = 0

		for pos in multi_color_tiles:
			var tile_value = multi_color_tiles[pos]

			# Check if we have a mapping for this tile value
			if multi_color_tile_mappings.has(tile_value):
				var atlas_coords = multi_color_tile_mappings[tile_value]
				target_tilemap_layer.set_cell(Vector2i(pos), multi_color_tile_source_id, atlas_coords)
				tiles_placed += 1

				# Debug: Show first few placements
				if tiles_placed <= 3:
					print("  DEBUG: Placed tile at (%d,%d) with source_id=%d, atlas=(%d,%d)" % [
						pos.x, pos.y, multi_color_tile_source_id, atlas_coords.x, atlas_coords.y
					])
			else:
				unmapped_tiles += 1

		print("DungeonPreview: Placed %d tiles, %d tiles unmapped" % [tiles_placed, unmapped_tiles])

		if unmapped_tiles > 0:
			print("  → Add mappings to 'Multi Color Tile Mappings' dictionary:")
			var unique_unmapped = {}
			for pos in multi_color_tiles:
				var tile_value = multi_color_tiles[pos]
				if not multi_color_tile_mappings.has(tile_value):
					unique_unmapped[tile_value] = true

			# Sort by tile value for consistent display
			var sorted_values = unique_unmapped.keys()
			sorted_values.sort()

			for tile_value in sorted_values:
				# Convert integer back to RGB for display (matches C++ format)
				var r = (tile_value >> 16) & 0xFF
				var g = (tile_value >> 8) & 0xFF
				var b = tile_value & 0xFF
				var color_name = _get_color_name(r, g, b)
				var color_hex = "#%02X%02X%02X" % [r, g, b]

				# Also show the actual Color from palette if available
				var actual_color_str = ""
				if owfc_color_palette.has(tile_value):
					var palette_color: Color = owfc_color_palette[tile_value]
					actual_color_str = " [Actual: #%02X%02X%02X]" % [
						int(palette_color.r * 255),
						int(palette_color.g * 255),
						int(palette_color.b * 255)
					]

				print("    %d -> Vector2i(x, y)  # %s RGB(%d,%d,%d) - %s%s" % [
					tile_value, color_hex, r, g, b, color_name, actual_color_str
				])

	# Standard floor/wall mode
	else:
		print("DungeonPreview: Populating TileMapLayer with %d floors and %d walls..." % [floor_positions.size(), wall_positions.size()])

		# Place floor tiles
		for pos in floor_positions:
			target_tilemap_layer.set_cell(Vector2i(pos), floor_tile_source_id, floor_tile_atlas_coords)

		# Place wall tiles
		for pos in wall_positions:
			target_tilemap_layer.set_cell(Vector2i(pos), wall_tile_source_id, wall_tile_atlas_coords)

	print("DungeonPreview: TileMapLayer populated successfully")

## Export preview as PNG
func export_as_png(path: String = "") -> int:
	if path.is_empty():
		path = export_path

	# Get viewport texture
	var img = get_viewport().get_texture().get_image()

	# Calculate bounds
	var rect = _get_tile_bounds_rect()

	# Crop to dungeon area (with some padding)
	var padding = tile_size * 2
	var crop_rect = Rect2i(
		int(rect.position.x - padding.x),
		int(rect.position.y - padding.y),
		int(rect.size.x + padding.x * 2),
		int(rect.size.y + padding.y * 2)
	)

	# Clamp to viewport bounds
	crop_rect = crop_rect.intersection(Rect2i(0, 0, img.get_width(), img.get_height()))

	# Create cropped image
	var cropped = img.get_region(crop_rect)

	# Scale if needed
	if export_scale != 1.0:
		var new_size = Vector2i(
			int(cropped.get_width() * export_scale),
			int(cropped.get_height() * export_scale)
		)
		cropped.resize(new_size.x, new_size.y, Image.INTERPOLATE_NEAREST)

	# Save
	var err = cropped.save_png(path)
	if err == OK:
		print("DungeonPreview: Exported preview to ", path)
	else:
		push_error("DungeonPreview: Failed to export preview: ", err)

	return err

# ============================================================================
# INTERNAL DRAWING METHODS
# ============================================================================

func _draw_empty_state() -> void:
	var font = ThemeDB.fallback_font
	var font_size = ThemeDB.fallback_font_size
	var text = "No dungeon data to display"
	var text_size = font.get_string_size(text, HORIZONTAL_ALIGNMENT_CENTER, -1, font_size)
	draw_string(font, Vector2(-text_size.x / 2, 0), text, HORIZONTAL_ALIGNMENT_CENTER, -1, font_size, Color.GRAY)

func _draw_single_view() -> void:
	# Apply LOD if needed
	var floors_to_draw = floor_positions
	var walls_to_draw = wall_positions

	if enable_lod and floor_positions.size() > lod_threshold:
		floors_to_draw = _apply_lod(floor_positions)
		walls_to_draw = _apply_lod(wall_positions)

	# Multi-color mode: draw each tile with its palette color
	if using_multi_color_mode and not owfc_color_palette.is_empty():
		var tiles_drawn = 0
		for pos in floors_to_draw:
			if multi_color_tiles.has(pos):
				var tile_id = multi_color_tiles[pos]
				var tile_color = owfc_color_palette.get(tile_id, floor_color)
				var rect = Rect2(Vector2(pos.x * tile_size.x, pos.y * tile_size.y), tile_size)
				draw_rect(rect, tile_color)
				tiles_drawn += 1

		if tiles_drawn == 0:
			print("DungeonPreview: Warning - Multi-color mode active but drew 0 tiles!")
			print("  - floors_to_draw size: ", floors_to_draw.size())
			print("  - multi_color_tiles size: ", multi_color_tiles.size())
			print("  - color_palette size: ", owfc_color_palette.size())
	else:
		# Standard two-color mode
		# Draw walls first (background)
		for pos in walls_to_draw:
			var rect = Rect2(Vector2(pos.x * tile_size.x, pos.y * tile_size.y), tile_size)
			draw_rect(rect, wall_color)

		# Draw floors (foreground)
		for pos in floors_to_draw:
			var rect = Rect2(Vector2(pos.x * tile_size.x, pos.y * tile_size.y), tile_size)
			draw_rect(rect, floor_color)

func _draw_split_view() -> void:
	var viewport_size = get_viewport_rect().size

	if split_direction == 0:  # Horizontal split
		var split_x = viewport_size.x / 2

		# Draw left side (original result)
		draw_set_transform(position, 0, Vector2.ONE)
		_draw_in_bounds(floor_positions, wall_positions, Rect2(0, 0, split_x, viewport_size.y))

		# Draw divider
		draw_line(Vector2(split_x, 0), Vector2(split_x, viewport_size.y), Color.WHITE, 2.0)

		# Draw right side (comparison result)
		draw_set_transform(Vector2(split_x, 0), 0, Vector2.ONE)
		_draw_in_bounds(floor_positions_b, wall_positions_b, Rect2(0, 0, split_x, viewport_size.y))

	else:  # Vertical split
		var split_y = viewport_size.y / 2

		# Draw top side (original result)
		draw_set_transform(position, 0, Vector2.ONE)
		_draw_in_bounds(floor_positions, wall_positions, Rect2(0, 0, viewport_size.x, split_y))

		# Draw divider
		draw_line(Vector2(0, split_y), Vector2(viewport_size.x, split_y), Color.WHITE, 2.0)

		# Draw bottom side (comparison result)
		draw_set_transform(Vector2(0, split_y), 0, Vector2.ONE)
		_draw_in_bounds(floor_positions_b, wall_positions_b, Rect2(0, 0, viewport_size.x, split_y))

	# Reset transform
	draw_set_transform(position, 0, Vector2.ONE)

func _draw_in_bounds(floors: PackedVector2Array, walls: PackedVector2Array, bounds: Rect2) -> void:
	# Draw walls
	for pos in walls:
		var rect = Rect2(Vector2(pos.x * tile_size.x, pos.y * tile_size.y), tile_size)
		if bounds.intersects(rect):
			draw_rect(rect, wall_color)

	# Draw floors
	for pos in floors:
		var rect = Rect2(Vector2(pos.x * tile_size.x, pos.y * tile_size.y), tile_size)
		if bounds.intersects(rect):
			draw_rect(rect, floor_color)

func _draw_grid() -> void:
	if floor_positions.is_empty():
		return

	var bounds_rect = _get_tile_bounds_rect()
	var min_x = int(bounds_rect.position.x / tile_size.x)
	var max_x = int((bounds_rect.position.x + bounds_rect.size.x) / tile_size.x) + 1
	var min_y = int(bounds_rect.position.y / tile_size.y)
	var max_y = int((bounds_rect.position.y + bounds_rect.size.y) / tile_size.y) + 1

	# Draw vertical lines
	for x in range(min_x, max_x + 1):
		var x_pos = x * tile_size.x
		draw_line(
			Vector2(x_pos, min_y * tile_size.y),
			Vector2(x_pos, max_y * tile_size.y),
			grid_color, 1.0
		)

	# Draw horizontal lines
	for y in range(min_y, max_y + 1):
		var y_pos = y * tile_size.y
		draw_line(
			Vector2(min_x * tile_size.x, y_pos),
			Vector2(max_x * tile_size.x, y_pos),
			grid_color, 1.0
		)

func _draw_coordinates() -> void:
	if floor_positions.is_empty():
		return

	var font = ThemeDB.fallback_font
	var font_size = 10
	var bounds_rect = _get_tile_bounds_rect()
	var min_x = int(bounds_rect.position.x / tile_size.x)
	var max_x = int((bounds_rect.position.x + bounds_rect.size.x) / tile_size.x)
	var min_y = int(bounds_rect.position.y / tile_size.y)
	var max_y = int((bounds_rect.position.y + bounds_rect.size.y) / tile_size.y)

	# Draw X coordinates
	for x in range(min_x, max_x + 1):
		if x % coordinate_interval == 0:
			var pos = Vector2(x * tile_size.x, min_y * tile_size.y - 5)
			draw_string(font, pos, str(x), HORIZONTAL_ALIGNMENT_CENTER, -1, font_size, Color.WHITE)

	# Draw Y coordinates
	for y in range(min_y, max_y + 1):
		if y % coordinate_interval == 0:
			var pos = Vector2(min_x * tile_size.x - 20, y * tile_size.y)
			draw_string(font, pos, str(y), HORIZONTAL_ALIGNMENT_RIGHT, -1, font_size, Color.WHITE)

func _draw_statistics() -> void:
	var font = ThemeDB.fallback_font
	var line_height = stats_font_size + 4

	# Build stats text
	var lines: Array[String] = []
	lines.append("=== Dungeon Statistics ===")
	lines.append("Floors: %d" % floor_positions.size())
	lines.append("Walls: %d" % wall_positions.size())

	if stats.has("map_width"):
		lines.append("Map Size: %dx%d" % [stats.map_width, stats.map_height])

	if generation_time_ms > 0:
		lines.append("Gen Time: %.2f ms" % generation_time_ms)

	# Add custom stats
	for key in stats.keys():
		if key not in ["map_width", "map_height", "floor_count", "wall_count"]:
			lines.append("%s: %s" % [key, str(stats[key])])

	# Calculate background rect
	var max_width = 0.0
	for line in lines:
		var text_size = font.get_string_size(line, HORIZONTAL_ALIGNMENT_LEFT, -1, stats_font_size)
		max_width = max(max_width, text_size.x)

	var bg_rect = Rect2(
		stats_position,
		Vector2(max_width + 20, lines.size() * line_height + 10)
	)

	# Draw background
	if stats_background:
		draw_rect(bg_rect, Color(0, 0, 0, 0.7))
		draw_rect(bg_rect, Color.WHITE, false, 1.0)

	# Draw text
	var y_offset = stats_position.y + 15
	for line in lines:
		draw_string(
			font,
			Vector2(stats_position.x + 10, y_offset),
			line,
			HORIZONTAL_ALIGNMENT_LEFT,
			-1,
			stats_font_size,
			Color.WHITE
		)
		y_offset += line_height

# ============================================================================
# HELPER METHODS
# ============================================================================

func _update_bounds() -> void:
	if floor_positions.is_empty():
		bounds_min = Vector2.ZERO
		bounds_max = Vector2.ZERO
		return

	bounds_min = floor_positions[0]
	bounds_max = floor_positions[0]

	for pos in floor_positions:
		bounds_min = bounds_min.min(pos)
		bounds_max = bounds_max.max(pos)

	for pos in wall_positions:
		bounds_min = bounds_min.min(pos)
		bounds_max = bounds_max.max(pos)

func _update_view_centering() -> void:
	if not center_view or floor_positions.is_empty():
		position = Vector2.ZERO
		return

	var center = Vector2(
		(bounds_min.x + bounds_max.x) / 2.0 * tile_size.x,
		(bounds_min.y + bounds_max.y) / 2.0 * tile_size.y
	)
	var viewport_center = get_viewport_rect().size / 2.0
	position = viewport_center - center

func _get_tile_bounds_rect() -> Rect2:
	if floor_positions.is_empty():
		return Rect2()

	return Rect2(
		Vector2(bounds_min.x * tile_size.x, bounds_min.y * tile_size.y),
		Vector2(
			(bounds_max.x - bounds_min.x + 1) * tile_size.x,
			(bounds_max.y - bounds_min.y + 1) * tile_size.y
		)
	)

func _get_visible_rect() -> Rect2:
	var bounds_rect = _get_tile_bounds_rect()
	var padding = tile_size * 2
	return Rect2(
		bounds_rect.position - padding,
		bounds_rect.size + padding * 2
	)

func _apply_lod(positions: PackedVector2Array) -> PackedVector2Array:
	if positions.size() <= max_visible_tiles:
		return positions

	# Simple LOD: skip every N tiles
	var skip_factor = ceili(float(positions.size()) / float(max_visible_tiles))
	var result: PackedVector2Array = PackedVector2Array()

	for i in range(0, positions.size(), skip_factor):
		result.append(positions[i])

	return result

# ============================================================================
# OVERLAPPING WFC HELPERS
# ============================================================================

func _load_owfc_seed_image() -> void:
	if owfc_seed_image_path.is_empty():
		owfc_seed_image = null
		owfc_seed_preview_texture = null
		owfc_extracted_colors.clear()
		owfc_color_palette.clear()
		return

	if not FileAccess.file_exists(owfc_seed_image_path):
		push_error("DungeonPreview: Seed image not found: ", owfc_seed_image_path)
		owfc_seed_image = null
		owfc_seed_preview_texture = null
		owfc_extracted_colors.clear()
		owfc_color_palette.clear()
		return

	# Load the image
	owfc_seed_image = Image.load_from_file(owfc_seed_image_path)

	if owfc_seed_image == null:
		push_error("DungeonPreview: Failed to load seed image: ", owfc_seed_image_path)
		owfc_seed_preview_texture = null
		owfc_extracted_colors.clear()
		owfc_color_palette.clear()
		return

	# Extract unique colors from seed image and build color palette
	_extract_colors_from_seed_image()

	# Create preview texture
	owfc_seed_preview_texture = ImageTexture.create_from_image(owfc_seed_image)
	print("DungeonPreview: Loaded seed image: ", owfc_seed_image_path, " (", owfc_seed_image.get_width(), "x", owfc_seed_image.get_height(), ")")
	print("DungeonPreview: Extracted %d unique colors from seed image" % owfc_extracted_colors.size())

func _draw_seed_preview() -> void:
	# Draw a larger preview of the seed image
	var preview_scale = 8.0  # Scale up small seed images
	var actual_size = Vector2(owfc_seed_image.get_width(), owfc_seed_image.get_height()) * preview_scale
	var preview_pos = Vector2(get_viewport_rect().size.x - actual_size.x - 20, 20)

	# Draw background with border
	var border_width = 3.0
	draw_rect(Rect2(preview_pos - Vector2(border_width, border_width),
		actual_size + Vector2(border_width * 2, border_width * 2)), Color.WHITE, false, border_width)
	draw_rect(Rect2(preview_pos, actual_size), Color(0.1, 0.1, 0.1))

	# Draw texture with nearest-neighbor filtering (no blur)
	if owfc_seed_preview_texture:
		draw_texture_rect(owfc_seed_preview_texture, Rect2(preview_pos, actual_size), false)

	# Label above the image
	var font = ThemeDB.fallback_font
	var label = "Seed: %dx%d" % [owfc_seed_image.get_width(), owfc_seed_image.get_height()]
	draw_string(font, preview_pos + Vector2(0, -8), label, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color.WHITE)

func _get_color_name(r: int, g: int, b: int) -> String:
	# Helper to identify common colors
	if r == 0 and g == 0 and b == 0:
		return "Black"
	if r == 255 and g == 255 and b == 255:
		return "White"

	# Check for dominant channel
	var max_channel = max(r, max(g, b))
	var min_channel = min(r, min(g, b))

	# Grayscale
	if max_channel - min_channel < 30:
		if max_channel < 85:
			return "Dark Gray"
		elif max_channel < 170:
			return "Gray"
		else:
			return "Light Gray"

	# Check which channel dominates
	if r > g + 50 and r > b + 50:
		if g > 100 or b > 100:
			return "Orange/Brown"
		return "Red"
	if g > r + 50 and g > b + 50:
		return "Green"
	if b > r + 50 and b > g + 50:
		if b > 200 and r < 100 and g < 100:
			return "Blue"
		return "Blue/Cyan"

	# Mixed colors
	if r > 150 and g > 150:
		return "Yellow"
	if r > 150 and b > 150:
		return "Magenta"
	if g > 150 and b > 150:
		return "Cyan"
	if r > 100 and g > 50 and g < 150 and b < 50:
		return "Brown"

	return "Mixed"

func _auto_generate_tile_mappings() -> void:
	if not using_multi_color_mode or multi_color_tiles.is_empty():
		push_warning("DungeonPreview: No multi-color data available. Generate a dungeon first!")
		return

	# Collect all unique tile values
	var unique_tiles: Dictionary = {}
	for pos in multi_color_tiles:
		var tile_value = multi_color_tiles[pos]
		unique_tiles[tile_value] = true

	# Sort for consistent mapping
	var sorted_tiles = unique_tiles.keys()
	sorted_tiles.sort()

	# Create new mappings dictionary (exported properties are read-only)
	var new_mappings: Dictionary = {}

	# Generate sequential atlas coordinates
	var current_x = auto_mapping_start_coords.x
	var current_y = auto_mapping_start_coords.y

	print("DungeonPreview: Auto-generating tile mappings...")
	print("  Direction: %s" % ("Horizontal" if auto_mapping_direction == 0 else "Vertical"))
	print("  Start: (%d, %d)" % [current_x, current_y])
	print("")

	for i in range(sorted_tiles.size()):
		var tile_value = sorted_tiles[i]

		# Create the mapping
		new_mappings[tile_value] = Vector2i(current_x, current_y)

		# Show what was mapped
		var r = (tile_value >> 16) & 0xFF
		var g = (tile_value >> 8) & 0xFF
		var b = tile_value & 0xFF
		var color_hex = "#%02X%02X%02X" % [r, g, b]
		var color_name = _get_color_name(r, g, b)

		print("  %d -> Vector2i(%d, %d)  # %s - %s" % [
			tile_value, current_x, current_y, color_hex, color_name
		])

		# Move to next atlas position
		if auto_mapping_direction == 0:  # Horizontal
			current_x += 1
		else:  # Vertical
			current_y += 1

	print("")
	print("DungeonPreview: Generated %d tile mappings!" % sorted_tiles.size())
	print("  → Make sure your tileset has tiles at these atlas coordinates")
	print("  → Click 'Populate Tilemap Button' to apply")

	# Assign the new dictionary to the exported property
	multi_color_tile_mappings = new_mappings

	# Trigger property update
	notify_property_list_changed()

func _extract_colors_from_seed_image() -> void:
	if not owfc_seed_image:
		return

	owfc_extracted_colors.clear()
	owfc_color_palette.clear()
	var unique_colors: Dictionary = {}  # Use dict as set for fast lookup

	var width = owfc_seed_image.get_width()
	var height = owfc_seed_image.get_height()

	# Scan all pixels and collect unique colors
	for y in range(height):
		for x in range(width):
			var pixel_color = owfc_seed_image.get_pixel(x, y)

			# Convert color to integer (same way C++ does it)
			var color_value = (int(pixel_color.r * 255) << 16) | \
			                  (int(pixel_color.g * 255) << 8) | \
			                  int(pixel_color.b * 255)

			# Store mapping from integer color value to actual color
			if not unique_colors.has(color_value):
				unique_colors[color_value] = true
				owfc_extracted_colors.append(pixel_color)
				owfc_color_palette[color_value] = pixel_color

	print("DungeonPreview: Found %d unique colors in seed image" % owfc_extracted_colors.size())

