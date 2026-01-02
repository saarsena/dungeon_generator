#ifndef GDWFC_H
#define GDWFC_H

#include <vector>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class GDTilingWFC : public RefCounted {
    GDCLASS(GDTilingWFC, RefCounted)

public:
    enum SymmetryType {
        SYMMETRY_X = 0,        // No rotation - 1 orientation
        SYMMETRY_I = 1,        // 2 orientations (vertical/horizontal)
        SYMMETRY_BACKSLASH = 2, // 2 orientations (diagonal)
        SYMMETRY_T = 3,        // 4 orientations (T-shape)
        SYMMETRY_L = 4,        // 4 orientations (L-shape)
        SYMMETRY_P = 5         // 8 orientations (fully asymmetric)
    };

private:
    int width;
    int height;
    int seed;
    bool periodic;

    // Store tile definitions and neighbor rules
    struct TileDefinition {
        int tile_id;
        PackedInt32Array tile_data;
        int tile_size;  // Width/height of the square tile
        SymmetryType symmetry;
        float weight;
    };

    struct NeighborRule {
        int tile1_id;
        int orientation1;
        int tile2_id;
        int orientation2;
    };

    std::vector<TileDefinition> tiles;
    std::vector<NeighborRule> neighbor_rules;

protected:
    static void _bind_methods();

public:
    GDTilingWFC();
    ~GDTilingWFC();

    // Configuration methods
    void set_size(int p_width, int p_height);
    void set_seed(int p_seed);
    void set_periodic(bool p_periodic);

    // Add a tile with its data, symmetry, and weight
    // tile_data should be a flat array representing a square tile
    // For a 2x2 tile: [top_left, top_right, bottom_left, bottom_right]
    void add_tile(int tile_id, PackedInt32Array tile_data, int tile_size, SymmetryType symmetry, float weight);

    // Add a neighbor rule specifying which tiles can be adjacent
    // orientation1/orientation2 refer to the rotation/reflection variant of each tile
    // For SYMMETRY_X: only orientation 0 exists
    // For SYMMETRY_I/BACKSLASH: orientations 0-1
    // For SYMMETRY_T/L: orientations 0-3
    // For SYMMETRY_P: orientations 0-7
    void add_neighbor_rule(int tile1_id, int orientation1, int tile2_id, int orientation2);

    // Run the WFC algorithm
    // Returns a flat PackedInt32Array of the generated output, or empty array on failure
    PackedInt32Array run();

    // Clear all tiles and rules
    void clear();

    // Get result dimensions (after run)
    int get_output_width() const;
    int get_output_height() const;
};

VARIANT_ENUM_CAST(GDTilingWFC::SymmetryType);

#endif // GDWFC_H
