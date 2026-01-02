# Changelog

## [Unreleased] - 2025-10-27

### Added
- **Overlapping WFC Generator**: Complete implementation of pattern-based procedural generation
  - Extract patterns from seed images (any pixel art)
  - Automatic color extraction and palette generation
  - Multi-color tile visualization with exact seed image colors
  - C++ implementation using fast-wfc library for high performance
  - Full GDScript API with `OverlappingWFCGenerator` and `OverlappingWFCResult` classes

- **DungeonPreview Enhancements**: Unified preview node now supports all four algorithms
  - Consolidated OverlappingWFCPreview functionality into main DungeonPreview node
  - Added algorithm switcher: WFC / Walker / BSP / Overlapping WFC
  - Larger seed image preview (8x scale) with dimensions display
  - Real-time color extraction from seed images
  - Multi-color tile rendering using seed image colors

- **TileMapLayer Integration**: Complete automated workflow for tilemap population
  - Auto-generate tile mappings from unique colors
  - Sequential atlas coordinate assignment (horizontal/vertical)
  - Multi-color tile support with color-to-atlas mapping
  - Manual populate button for explicit control
  - Auto-populate option for immediate results
  - Helpful console output showing color names and hex codes
  - Debug output for tilemap state verification

- **fast-wfc Library**: Integrated Overlapping WFC implementation
  - Added as git submodule in `fast-wfc/`
  - Compiled directly into extension (no external dependencies)
  - Full pattern extraction and constraint solving

### Removed
- **FOV System**: Removed field-of-view system that was erroneously included in the project
  - Removed FOV references from CMakeLists.txt
  - Removed FOV mentions from register_types.cpp
  - Updated all documentation to reflect removal

- **Duplicate Preview Nodes**: Removed OverlappingWFCPreview node (merged into DungeonPreview)
  - All Overlapping WFC functionality now in unified DungeonPreview node
  - Removed old wfc addon directory (`addons/wfc/`)

### Changed
- **Addon Directory**: Renamed from `addons/wfc/` to `addons/dungeon_generator/`
  - More descriptive name reflecting all generator types
  - Updated all paths in CMakeLists.txt and documentation

- **Plugin Configuration**: Renamed plugin files for consistency
  - `wfc.gdextension` → `dungeon_generator.gdextension`
  - Updated plugin.cfg with new addon name

- **Overlapping WFC Defaults**: Optimized settings for higher success rate
  - pattern_size: 3 → 2 (more flexible)
  - symmetry: 2 → 8 (maximum variety)
  - output_size: 48x48 → 20x20 (fewer constraints)
  - periodic_input: false → true (more patterns extracted)
  - ground_mode: true → false (prevents crashes with small images)

### Fixed
- **Overlapping WFC Ground Mode Crash**: Fixed assertion failure with small seed images
  - Ground mode now disabled by default with warning in inspector
  - Added validation to prevent wraparound pattern extraction issues
  - Better error messages when WFC fails

- **Dictionary Read-Only Error**: Fixed tile mapping generation
  - Exported Dictionary properties are read-only in Godot
  - Auto-generate mappings now creates new Dictionary and reassigns

- **Color Extraction**: Fixed RGB conversion from integer color values
  - Proper bit shifting for R/G/B channel extraction
  - Hex color codes now display correctly in console
  - Color names accurately identify tile colors

- **BSP Hallway Floors**: Corridor tiles now properly included in `get_floor_positions()`
  - Previously corridors had no floor tiles (returned as empty/void)
  - Now corridors are in both `get_floor_positions()` and `get_corridor_positions()`
  - Makes it easy to place floor tiles uniformly across all walkable areas

- **Plugin System**: Fixed plugin.cfg and created proper plugin.gd
  - Plugin script now properly registers DungeonPreview node in editor
  - Fixed script path to use relative path instead of absolute

### Changed
- **Build Output Structure**: Simplified from scattered files to unified deploy directory
  - Old: `build/addons/wfc/bin/{platform}/` (binaries only, manual file copying)
  - New: `build/deploy/addons/wfc/` (complete addon, single copy command)
- **Documentation**: Updated all build instructions to reflect new deploy system
  - README.md: Updated quick start with deploy directory
  - BUILD.md: Complete rewrite with deploy system instructions
  - CLAUDE.md: Updated for future AI assistance
  - BSPDungeonGenerator.md: Added note about corridor floor behavior

### Developer Notes
- Post-build commands automatically copy addon files
- Platform-specific output naming properly handles Debug vs Release
- Build system prints clear instructions for deployment

## [1.0.1] - Previous

### Features
- WFC v2 API with connection-based tiles
- BSP dungeon generator
- Walker cave generator
- DungeonPreview visual editor node
