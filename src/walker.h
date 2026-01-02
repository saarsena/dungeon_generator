// walker.h - Walker dungeon generation for Godot
#ifndef WALKER_H
#define WALKER_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include <vector>
#include <unordered_set>
#include <random>

using namespace godot;

// ============================================================================
// WalkerResult - Holds the result of walker generation
// ============================================================================
class WalkerResult : public RefCounted {
    GDCLASS(WalkerResult, RefCounted)

private:
    PackedVector2Array floor_positions;
    PackedVector2Array wall_positions;
    int map_width;
    int map_height;

protected:
    static void _bind_methods();

public:
    WalkerResult();
    ~WalkerResult();

    // Result queries
    PackedVector2Array get_floor_positions() const { return floor_positions; }
    PackedVector2Array get_wall_positions() const { return wall_positions; }
    int get_map_width() const { return map_width; }
    int get_map_height() const { return map_height; }

    // Query TileMapLayer for positions matching specific atlas coords
    PackedVector2Array get_tilemap_positions_with_atlas(Object* tilemap_layer, Vector2i atlas_coords, int source_id = 0) const;

    // Statistics
    Dictionary get_statistics() const;

    // Internal setters (used by generator)
    void _set_result_data(PackedVector2Array floors, PackedVector2Array walls, int width, int height);
};

// ============================================================================
// WalkerDungeonGenerator - Main generator class
// ============================================================================
class WalkerDungeonGenerator : public RefCounted {
    GDCLASS(WalkerDungeonGenerator, RefCounted)

private:
    // Configuration parameters
    bool allow_overlap;
    int min_hall;
    int max_hall;
    int room_dim;
    int total_floor_count;
    int seed;
    bool use_seed;

    // Generation state
    Vector2i map_size;
    std::mt19937 rng;

    // Internal data structures
    struct Room {
        Vector2i center;
        int width, height;
    };

    struct Walker {
        Vector2i position;
        Vector2i direction;
        Vector2i last_direction;
    };

    struct Vector2iHash {
        size_t operator()(const Vector2i& v) const {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };

    std::unordered_set<Vector2i, Vector2iHash> floor_tiles;
    std::unordered_set<Vector2i, Vector2iHash> walls;
    std::vector<Room> rooms;
    std::vector<Walker> walkers;

    // Internal methods
    void calculate_map_size();
    Vector2i random_direction(Vector2i current = Vector2i(0, 0));
    int randi_range(int from, int to);
    bool room_overlaps_existing(Vector2i center, int width, int height);
    void simulate_walkers();
    void place_organic_room(Vector2i center, int base_width, int base_height);
    bool try_place_room(Vector2i center);
    void spawn_walker();
    void generate_walls();

protected:
    static void _bind_methods();

public:
    WalkerDungeonGenerator();
    ~WalkerDungeonGenerator();

    // Configuration
    void set_allow_overlap(bool p_allow_overlap);
    bool get_allow_overlap() const { return allow_overlap; }

    void set_min_hall(int p_min_hall);
    int get_min_hall() const { return min_hall; }

    void set_max_hall(int p_max_hall);
    int get_max_hall() const { return max_hall; }

    void set_room_dim(int p_room_dim);
    int get_room_dim() const { return room_dim; }

    void set_total_floor_count(int p_total_floor_count);
    int get_total_floor_count() const { return total_floor_count; }

    void set_seed(int p_seed);
    int get_seed() const { return seed; }

    void set_use_seed(bool p_use_seed);
    bool get_use_seed() const { return use_seed; }

    // Generation
    Ref<WalkerResult> generate();
};

#endif // WALKER_H
