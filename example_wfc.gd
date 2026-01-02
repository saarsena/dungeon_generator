extends Node

## Example demonstrating how to use the GDTilingWFC GDExtension
##
## This example creates a simple 2-tile pattern (like a checkerboard)
## and generates a random tiling using the Wave Function Collapse algorithm.

func _ready():
	# Create a new WFC instance
	var wfc = GDTilingWFC.new()

	# Configure the output size (in tiles)
	wfc.set_size(20, 20)
	wfc.set_seed(12345)  # Use a fixed seed for reproducible results
	wfc.set_periodic(false)  # Set to true for seamless tiling

	# Define tiles
	# Each tile is a small grid of values (e.g., colors, tile IDs, etc.)
	# For this simple example, we use 1x1 tiles (single values)

	# Tile 0: Black tile
	var tile0_data = PackedInt32Array([0])
	wfc.add_tile(0, tile0_data, 1, GDTilingWFC.SYMMETRY_X, 1.0)

	# Tile 1: White tile
	var tile1_data = PackedInt32Array([1])
	wfc.add_tile(1, tile1_data, 1, GDTilingWFC.SYMMETRY_X, 1.0)

	# Define neighbor rules (adjacency constraints)
	# Format: add_neighbor_rule(tile1_id, orientation1, tile2_id, orientation2)
	# orientation1/2 refer to which rotation variant of the tile, not direction
	# Since we used SYMMETRY_X (no rotation), only orientation 0 exists

	# Allow all tiles to be adjacent to each other
	wfc.add_neighbor_rule(0, 0, 0, 0)  # Tile 0 can be next to tile 0
	wfc.add_neighbor_rule(0, 0, 1, 0)  # Tile 0 can be next to tile 1
	wfc.add_neighbor_rule(1, 0, 0, 0)  # Tile 1 can be next to tile 0
	wfc.add_neighbor_rule(1, 0, 1, 0)  # Tile 1 can be next to tile 1

	# Run the WFC algorithm
	print("Running WFC algorithm...")
	var result = wfc.run()

	if result.size() > 0:
		print("WFC succeeded!")
		print("Output dimensions: ", wfc.get_output_width(), "x", wfc.get_output_height())

		# Process the result
		# result is a flat PackedInt32Array of size (width * height)
		print_pattern(result, wfc.get_output_width(), wfc.get_output_height())

		# You can use this result to:
		# - Populate a TileMap
		# - Generate textures
		# - Create procedural levels
		# Example: generate_tilemap(result, wfc.get_output_width(), wfc.get_output_height())
	else:
		print("WFC failed to find a solution")

func print_pattern(data: PackedInt32Array, width: int, height: int):
	"""Print the first few rows of the generated pattern"""
	print("Pattern preview (first 10 rows):")
	for y in range(min(10, height)):
		var row = ""
		for x in range(min(40, width)):
			var value = data[y * width + x]
			row += str(value) if value == 0 else "#"
		print(row)

func generate_tilemap(data: PackedInt32Array, width: int, height: int):
	"""Generate a TileMap from the WFC output"""
	# Assuming you have a TileMap node as a child
	var tilemap = $TileMap

	for y in range(height):
		for x in range(width):
			var tile_id = data[y * width + x]
			# Set the cell in the TileMap
			# Adjust the parameters based on your TileMap configuration
			tilemap.set_cell(0, Vector2i(x, y), 0, Vector2i(tile_id, 0))

## Advanced example with more complex tiles
func advanced_example():
	var wfc = GDTilingWFC.new()
	wfc.set_size(15, 15)
	wfc.set_seed(randi())
	wfc.set_periodic(true)

	# Define 2x2 tiles with different patterns
	# Tile 0: Solid
	var tile0 = PackedInt32Array([1, 1, 1, 1])
	wfc.add_tile(0, tile0, 2, GDTilingWFC.SYMMETRY_X, 0.5)

	# Tile 1: Horizontal stripe (uses SYMMETRY_I for 2 rotations)
	var tile1 = PackedInt32Array([0, 0, 1, 1])
	wfc.add_tile(1, tile1, 2, GDTilingWFC.SYMMETRY_I, 1.0)

	# Tile 2: Corner (uses SYMMETRY_L for 4 rotations)
	var tile2 = PackedInt32Array([1, 0, 1, 1])
	wfc.add_tile(2, tile2, 2, GDTilingWFC.SYMMETRY_L, 1.0)

	# Add neighbor rules to create interesting patterns
	# This requires careful consideration of how tiles connect
	# For now, allow all connections between all tile orientations
	# Note: Tile 0 has 1 orientation (SYMMETRY_X)
	#       Tile 1 has 2 orientations (SYMMETRY_I)
	#       Tile 2 has 4 orientations (SYMMETRY_L)
	for t1 in range(3):
		for t2 in range(3):
			# Get max orientations for each tile based on symmetry
			var max_o1 = 1 if t1 == 0 else (2 if t1 == 1 else 4)
			var max_o2 = 1 if t2 == 0 else (2 if t2 == 1 else 4)
			for o1 in range(max_o1):
				for o2 in range(max_o2):
					wfc.add_neighbor_rule(t1, o1, t2, o2)

	var result = wfc.run()
	if result.size() > 0:
		print("Advanced WFC succeeded!")
		print_pattern(result, wfc.get_output_width(), wfc.get_output_height())
