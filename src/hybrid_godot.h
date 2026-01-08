#ifndef HYBRID_GODOT_H
#define HYBRID_GODOT_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include "DungeonBuilder.h"

using namespace godot;

class HybridResult : public RefCounted {
    GDCLASS(HybridResult, RefCounted)

private:
    // Stored data
    Array rooms;
    Array links;
    PackedVector2Array floors;
    PackedVector2Array walls;
    
    int grid_width;
    int grid_height;
    int tile_w;
    int tile_h;

protected:
    static void _bind_methods();

public:
    HybridResult();
    
    // Setters (used by generator)
    void set_data(const std::vector<RoomObj>& p_rooms, 
                 const std::vector<Link>& p_links,
                 const std::vector<Point>& p_floors,
                 const std::vector<Point>& p_walls,
                 const GenSettings& p_cfg);

    // Getters (exposed to Godot)
    Array get_rooms() const;
    Array get_links() const;
    PackedVector2Array get_floors() const;
    PackedVector2Array get_walls() const;
    
    int get_total_tiles() const;
    int get_grid_width() const;
    int get_grid_height() const;
    int get_tile_w() const;
    int get_tile_h() const;
};

class HybridDungeonGenerator : public RefCounted {
    GDCLASS(HybridDungeonGenerator, RefCounted)

private:
    GenSettings settings;

protected:
    static void _bind_methods();

public:
    HybridDungeonGenerator();

    // Configuration
    void set_room_count(int count);
    int get_room_count() const;

    void set_spread_radius(float radius);
    float get_spread_radius() const;

    void set_walker_count(int count);
    int get_walker_count() const;

    void set_grid_size(int width, int height);
    void set_grid_width(int width);
    void set_grid_height(int height);
    int get_grid_width() const;
    int get_grid_height() const;

    void set_tile_size(int w, int h);
    void set_tile_w(int w);
    void set_tile_h(int h);
    int get_tile_w() const;
    int get_tile_h() const;

    void set_seed(int seed);
    int get_seed() const;

    // Execution
    Ref<HybridResult> generate();
};

#endif // HYBRID_GODOT_H
