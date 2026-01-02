@tool
extends EditorPlugin

const PLUGIN_NAME = "Dungeon Generator"
const DungeonPreviewScript = preload("res://addons/dungeon_generator/DungeonPreview.gd")

func _enter_tree() -> void:
	print("DungeonGenerator Plugin: Loading...")

	# Add custom node types to the editor
	add_custom_type(
		"DungeonPreview",
		"Node2D",
		DungeonPreviewScript,
		null  # No icon for now
	)

	print("DungeonGenerator Plugin: Loaded successfully")
	print("  - DungeonPreview node available (supports WFC, Walker, BSP, and Overlapping WFC)")
	print("  - BSPDungeonGenerator class available")
	print("  - WalkerDungeonGenerator class available")
	print("  - GDTilingWFCv2 class available")
	print("  - OverlappingWFCGenerator class available")


func _exit_tree() -> void:
	print("DungeonGenerator Plugin: Unloading...")

	# Remove custom types
	remove_custom_type("DungeonPreview")

	print("DungeonGenerator Plugin: Unloaded")


func _get_plugin_name() -> String:
	return PLUGIN_NAME


func _get_icon(path: String) -> Texture2D:
	if FileAccess.file_exists(path):
		return load(path)
	return null
