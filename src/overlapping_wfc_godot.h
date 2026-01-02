#ifndef OVERLAPPING_WFC_GODOT_H
#define OVERLAPPING_WFC_GODOT_H

#include <vector>
#include <map>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Forward declarations
class OverlappingWFCResult;
class OverlappingWFCGenerator;

// ============================================================================
// OverlappingWFCResult - Result object for Overlapping WFC generation
// ============================================================================
class OverlappingWFCResult : public RefCounted {
    GDCLASS(OverlappingWFCResult, RefCounted)

private:
    PackedInt32Array pattern_output;    // Raw WFC pattern output
    PackedInt32Array tile_output;       // Converted to tile IDs (if stamp mapping enabled)
    PackedInt32Array expanded_output;   // Expanded with stamps (if enabled)
    int output_width;
    int output_height;
    int expanded_width;
    int expanded_height;
    bool has_stamps;
    int stamp_size;
    bool success;
    String failure_reason;

protected:
    static void _bind_methods();

public:
    OverlappingWFCResult();
    ~OverlappingWFCResult();

    // Result queries
    bool is_success() const { return success; }
    String get_failure_reason() const { return failure_reason; }

    // Dimensions
    int get_output_width() const { return output_width; }
    int get_output_height() const { return output_height; }
    int get_expanded_width() const { return expanded_width; }
    int get_expanded_height() const { return expanded_height; }

    // Tile access
    int get_pattern_at(int x, int y) const;
    int get_tile_at(int x, int y) const;
    int get_expanded_tile_at(int x, int y) const;
    PackedInt32Array get_pattern_output() const { return pattern_output; }
    PackedInt32Array get_tile_output() const { return tile_output; }
    PackedInt32Array get_expanded_output() const { return expanded_output; }

    // Helper methods for dungeon generation
    PackedVector2Array get_floor_positions(int floor_tile_value = 0) const;
    PackedVector2Array get_wall_positions(int wall_tile_value = 1) const;
    Dictionary get_statistics() const;

    // Internal setters (used by generator)
    void _set_pattern_data(PackedInt32Array patterns, int width, int height);
    void _set_tile_data(PackedInt32Array tiles, int width, int height);
    void _set_expanded_data(PackedInt32Array tiles, int width, int height, int p_stamp_size);
    void _set_failure(String reason);
};

// ============================================================================
// OverlappingWFCGenerator - Main Overlapping WFC generator class
// ============================================================================
class OverlappingWFCGenerator : public RefCounted {
    GDCLASS(OverlappingWFCGenerator, RefCounted)

private:
    // Generation parameters
    Ref<Image> seed_image;
    int output_width;
    int output_height;
    int pattern_size;
    int symmetry;
    int seed;
    bool use_seed;
    bool periodic_input;
    bool periodic_output;
    bool ground_mode;

    // Stamp system parameters
    bool use_stamps;
    int stamp_size;
    Dictionary pattern_to_tile_map;  // Maps color values to tile IDs
    Dictionary tile_stamps;           // Maps tile IDs to stamp patterns (PackedInt32Array)

    // Debug mode
    bool debug_mode;

protected:
    static void _bind_methods();

public:
    OverlappingWFCGenerator();
    ~OverlappingWFCGenerator();

    // ========================================================================
    // Configuration - Basic Settings
    // ========================================================================
    void set_seed_image(const Ref<Image>& p_image);
    Ref<Image> get_seed_image() const { return seed_image; }

    void set_output_size(int width, int height);
    void set_output_width(int width);
    void set_output_height(int height);
    int get_output_width() const { return output_width; }
    int get_output_height() const { return output_height; }

    void set_pattern_size(int size);
    int get_pattern_size() const { return pattern_size; }

    void set_symmetry(int p_symmetry);
    int get_symmetry() const { return symmetry; }

    void set_seed(int p_seed);
    int get_seed() const { return seed; }

    void set_use_seed(bool p_use);
    bool get_use_seed() const { return use_seed; }

    void set_periodic_input(bool periodic);
    bool get_periodic_input() const { return periodic_input; }

    void set_periodic_output(bool periodic);
    bool get_periodic_output() const { return periodic_output; }

    void set_ground_mode(bool enabled);
    bool get_ground_mode() const { return ground_mode; }

    // ========================================================================
    // Stamp System Configuration
    // ========================================================================
    void enable_stamps(bool enabled);
    bool get_stamps_enabled() const { return use_stamps; }

    void set_stamp_size(int size);
    int get_stamp_size() const { return stamp_size; }

    // Map a pattern color to a tile ID
    // color_value: The color value from the pattern (e.g., 0x000000 for black)
    // tile_id: The tile ID it should map to (e.g., 0 for floor, 1 for wall)
    void add_pattern_to_tile_mapping(int color_value, int tile_id);

    // Set the stamp pattern for a tile ID (e.g., 3x3 pattern of floor/wall)
    // tile_id: The tile ID
    // stamp_pattern: Flat array of stamp values (e.g., [0,0,0, 0,0,0, 0,0,0] for 3x3 floor)
    // stamp_width/height: Dimensions of the stamp
    void set_tile_stamp(int tile_id, const PackedInt32Array& stamp_pattern, int stamp_width, int stamp_height);

    // Clear all mappings
    void clear_pattern_mappings();
    void clear_tile_stamps();

    // ========================================================================
    // Quick Setup Helpers
    // ========================================================================
    // Setup default black/white to floor/wall mapping
    void setup_default_dungeon_mapping();

    // Setup default 3x3 stamps for floor (0) and wall (1)
    void setup_default_dungeon_stamps();

    // ========================================================================
    // Generation
    // ========================================================================
    Ref<OverlappingWFCResult> generate();

    // ========================================================================
    // Utility
    // ========================================================================
    void enable_debug(bool enabled);
    bool get_debug_enabled() const { return debug_mode; }

    Dictionary get_configuration_info() const;
};

#endif // OVERLAPPING_WFC_GODOT_H
