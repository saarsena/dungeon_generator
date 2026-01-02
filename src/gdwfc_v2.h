#ifndef GDWFC_V2_H
#define GDWFC_V2_H

#include <vector>
#include <map>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Forward declarations
class WFCConfiguration;
class WFCResult;

// ============================================================================
// WFCResult - Enhanced result object with helper methods
// ============================================================================
class WFCResult : public RefCounted {
    GDCLASS(WFCResult, RefCounted)

private:
    PackedInt32Array wfc_tiles;  // Raw WFC output (tile IDs)
    PackedInt32Array expanded_tiles;  // Expanded with stamps (if enabled)
    int wfc_width;
    int wfc_height;
    int expanded_width;
    int expanded_height;
    bool has_stamps;
    int stamp_size;
    bool success;
    String failure_reason;
    Vector2i failure_position;

protected:
    static void _bind_methods();

public:
    WFCResult();
    ~WFCResult();

    // Result queries
    bool is_success() const { return success; }
    String get_failure_reason() const { return failure_reason; }
    Vector2i get_failure_position() const { return failure_position; }

    // Dimensions
    int get_wfc_width() const { return wfc_width; }
    int get_wfc_height() const { return wfc_height; }
    int get_expanded_width() const { return expanded_width; }
    int get_expanded_height() const { return expanded_height; }

    // Tile access
    int get_wfc_tile_at(int x, int y) const;
    int get_expanded_tile_at(int x, int y) const;
    PackedInt32Array get_wfc_tiles() const { return wfc_tiles; }
    PackedInt32Array get_expanded_tiles() const { return expanded_tiles; }

    // Helper methods for dungeon generation
    PackedVector2Array get_floor_positions(int floor_tile_value = 0) const;
    PackedVector2Array get_wall_positions(int wall_tile_value = 1) const;
    Dictionary get_tile_distribution() const;

    // Internal setters (used by WFC generator)
    void _set_wfc_data(PackedInt32Array tiles, int width, int height);
    void _set_expanded_data(PackedInt32Array tiles, int width, int height, int p_stamp_size);
    void _set_failure(String reason, Vector2i position);
};

// ============================================================================
// WFCConfiguration - Reusable tile definitions and rules
// ============================================================================
class WFCConfiguration : public RefCounted {
    GDCLASS(WFCConfiguration, RefCounted)

public:
    enum SymmetryType {
        SYMMETRY_X = 0,        // No rotation - 1 orientation
        SYMMETRY_I = 1,        // 2 orientations (vertical/horizontal)
        SYMMETRY_BACKSLASH = 2, // 2 orientations (diagonal)
        SYMMETRY_T = 3,        // 4 orientations (T-shape)
        SYMMETRY_L = 4,        // 4 orientations (L-shape)
        SYMMETRY_P = 5         // 8 orientations (fully asymmetric)
    };

    struct TileDefinition {
        int tile_id;
        PackedInt32Array tile_data;
        int tile_size;
        SymmetryType symmetry;
        float weight;

        // Connection-based system
        bool has_connections;
        bool connect_left;
        bool connect_up;
        bool connect_right;
        bool connect_down;
        bool borders_all;  // Special case: can border any tile

        // Stamp system (3x3 or custom)
        bool has_stamp;
        PackedInt32Array stamp_pattern;  // Flat array: [row0_col0, row0_col1, ..., row2_col2]
        int stamp_width;
        int stamp_height;
    };

    struct NeighborRule {
        int tile1_id;
        int orientation1;
        int tile2_id;
        int orientation2;
    };

private:
    std::vector<TileDefinition> tiles;
    std::vector<NeighborRule> neighbor_rules;
    bool use_connection_system;
    bool rules_auto_generated;
    int stamp_size;

protected:
    static void _bind_methods();

public:
    WFCConfiguration();
    ~WFCConfiguration();

    // ========================================================================
    // Configuration modes
    // ========================================================================
    void enable_connection_system(bool enabled = true);
    void set_stamp_size(int size);  // 0 = no stamps, 3 = 3x3, etc.

    // ========================================================================
    // Add tiles (traditional method)
    // ========================================================================
    void add_tile(int tile_id, PackedInt32Array tile_data, int tile_size,
                  SymmetryType symmetry, float weight);

    // ========================================================================
    // Add tiles with connections (simplified method)
    // ========================================================================
    void add_connected_tile(int tile_id, Dictionary connections, float weight = 1.0,
                           SymmetryType symmetry = SYMMETRY_X);
    // connections = {"left": true, "right": true, "up": false, "down": false}

    // ========================================================================
    // Add tile with stamp pattern
    // ========================================================================
    void set_tile_stamp(int tile_id, PackedInt32Array stamp_pattern, int stamp_width, int stamp_height);

    // ========================================================================
    // Neighbor rules
    // ========================================================================
    void add_neighbor_rule(int tile1_id, int orientation1, int tile2_id, int orientation2);
    void auto_generate_neighbor_rules();  // Generate from connections
    void add_border_all_tile(int tile_id);  // Tile that can border anything

    // ========================================================================
    // Validation and debugging
    // ========================================================================
    bool validate_rules() const;
    Dictionary get_tile_info(int tile_id) const;
    int get_tile_count() const { return tiles.size(); }
    int get_rule_count() const { return neighbor_rules.size(); }

    // Clear
    void clear();
    void clear_rules_only();

    // Internal access (for WFC generator)
    const std::vector<TileDefinition>& get_tiles() const { return tiles; }
    const std::vector<NeighborRule>& get_rules() const { return neighbor_rules; }
    int get_stamp_size() const { return stamp_size; }
};

// ============================================================================
// GDTilingWFC - Main WFC generator (improved version)
// ============================================================================
class GDTilingWFCv2 : public RefCounted {
    GDCLASS(GDTilingWFCv2, RefCounted)

public:
    typedef WFCConfiguration::SymmetryType SymmetryType;

private:
    int width;
    int height;
    int seed;
    bool periodic;

    Ref<WFCConfiguration> config;
    bool debug_mode;

protected:
    static void _bind_methods();

public:
    GDTilingWFCv2();
    GDTilingWFCv2(Ref<WFCConfiguration> p_config);
    ~GDTilingWFCv2();

    // ========================================================================
    // Configuration (basic settings)
    // ========================================================================
    void set_size(int p_width, int p_height);
    void set_seed(int p_seed);
    void set_periodic(bool p_periodic);
    void set_configuration(Ref<WFCConfiguration> p_config);
    void enable_debug(bool enabled);

    // ========================================================================
    // Quick setup methods (for simple use cases)
    // ========================================================================
    void use_connection_system();
    void add_connected_tile(int tile_id, Dictionary connections, float weight = 1.0);
    void set_tile_stamp(int tile_id, PackedInt32Array stamp_pattern, int stamp_width, int stamp_height);
    void auto_generate_rules();

    // ========================================================================
    // Traditional methods (backward compatible)
    // ========================================================================
    void add_tile(int tile_id, PackedInt32Array tile_data, int tile_size,
                  SymmetryType symmetry, float weight);
    void add_neighbor_rule(int tile1_id, int orientation1, int tile2_id, int orientation2);

    // ========================================================================
    // Run WFC algorithm
    // ========================================================================
    Ref<WFCResult> run();

    // ========================================================================
    // Utility
    // ========================================================================
    void clear();
    Ref<WFCConfiguration> get_configuration() const { return config; }
};

VARIANT_ENUM_CAST(WFCConfiguration::SymmetryType);

#endif // GDWFC_V2_H
