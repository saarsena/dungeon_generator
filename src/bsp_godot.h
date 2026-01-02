#ifndef BSP_GODOT_H
#define BSP_GODOT_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <vector>
#include <random>

using namespace godot;

struct BSPVec2i {
    int x, y;
    BSPVec2i() : x(0), y(0) {}
    BSPVec2i(int x, int y) : x(x), y(y) {}
    bool operator==(const BSPVec2i& other) const { return x == other.x && y == other.y; }
    bool operator<(const BSPVec2i& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

struct BSPRect2i {
    BSPVec2i position;
    BSPVec2i size;
    BSPVec2i center() const {
        return { position.x + size.x / 2, position.y + size.y / 2 };
    }
};

class BSPLeaf {
public:
    BSPRect2i rect;
    BSPRect2i room;
    bool has_room = false;
    BSPLeaf* left = nullptr;
    BSPLeaf* right = nullptr;

    explicit BSPLeaf(const BSPRect2i& rect) : rect(rect) {}
    ~BSPLeaf() {
        delete left;
        delete right;
    }

    bool split(int min_room_size, std::mt19937& rng);
};

class BSPResult : public RefCounted {
    GDCLASS(BSPResult, RefCounted)

private:
    PackedVector2Array floor_positions;
    PackedVector2Array wall_positions;
    PackedVector2Array corridor_positions;

protected:
    static void _bind_methods();

public:
    BSPResult();
    ~BSPResult();

    void set_floor_positions(const PackedVector2Array& positions);
    void set_wall_positions(const PackedVector2Array& positions);
    void set_corridor_positions(const PackedVector2Array& positions);

    PackedVector2Array get_floor_positions() const;
    PackedVector2Array get_wall_positions() const;
    PackedVector2Array get_corridor_positions() const;
    int get_floor_count() const;
};

class BSPDungeonGenerator : public RefCounted {
    GDCLASS(BSPDungeonGenerator, RefCounted)

private:
    int map_width;
    int map_height;
    int min_room_size;
    int max_room_size;
    int max_splits;
    int room_padding;
    bool use_seed;
    int seed;
    std::mt19937 rng;

    std::vector<BSPLeaf*> leaves;
    std::vector<BSPRect2i> rooms;
    std::vector<BSPVec2i> corridors;

    void connect_rooms(BSPLeaf* node);
    BSPVec2i get_representative_point(BSPLeaf* leaf);
    void create_corridor(BSPVec2i a, BSPVec2i b);
    int randi_range(int from, int to);

protected:
    static void _bind_methods();

public:
    BSPDungeonGenerator();
    ~BSPDungeonGenerator();

    void set_map_size(int width, int height);
    void set_room_size_range(int min_size, int max_size);
    void set_max_splits(int splits);
    void set_room_padding(int padding);
    void set_use_seed(bool enabled);
    void set_seed(int seed_value);

    int get_map_width() const { return map_width; }
    int get_map_height() const { return map_height; }
    int get_min_room_size() const { return min_room_size; }
    int get_max_room_size() const { return max_room_size; }
    int get_max_splits() const { return max_splits; }
    int get_room_padding() const { return room_padding; }
    bool get_use_seed() const { return use_seed; }
    int get_seed() const { return seed; }

    Ref<BSPResult> generate();
};

#endif // BSP_GODOT_H
