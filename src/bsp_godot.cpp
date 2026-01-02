#include "bsp_godot.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <set>
#include <algorithm>

// BSPLeaf implementation
bool BSPLeaf::split(int min_room_size, std::mt19937& rng) {
    if (left || right)
        return false;

    int min_split_size = min_room_size + 2;
    bool can_split_h = rect.size.x >= min_split_size * 2;
    bool can_split_v = rect.size.y >= min_split_size * 2;

    if (!can_split_h && !can_split_v)
        return false;

    bool split_h = can_split_h && (!can_split_v || std::uniform_real_distribution<>(0, 1)(rng) < 0.5);

    if (split_h) {
        int split_x = std::uniform_int_distribution<>(min_split_size, rect.size.x - min_split_size)(rng);
        left = new BSPLeaf({rect.position, {split_x, rect.size.y}});
        right = new BSPLeaf({{rect.position.x + split_x, rect.position.y}, {rect.size.x - split_x, rect.size.y}});
    } else {
        int split_y = std::uniform_int_distribution<>(min_split_size, rect.size.y - min_split_size)(rng);
        left = new BSPLeaf({rect.position, {rect.size.x, split_y}});
        right = new BSPLeaf({{rect.position.x, rect.position.y + split_y}, {rect.size.x, rect.size.y - split_y}});
    }

    return true;
}

// BSPResult implementation
BSPResult::BSPResult() {
}

BSPResult::~BSPResult() {
}

void BSPResult::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_floor_positions"), &BSPResult::get_floor_positions);
    ClassDB::bind_method(D_METHOD("get_wall_positions"), &BSPResult::get_wall_positions);
    ClassDB::bind_method(D_METHOD("get_corridor_positions"), &BSPResult::get_corridor_positions);
    ClassDB::bind_method(D_METHOD("get_floor_count"), &BSPResult::get_floor_count);
}

void BSPResult::set_floor_positions(const PackedVector2Array& positions) {
    floor_positions = positions;
}

void BSPResult::set_wall_positions(const PackedVector2Array& positions) {
    wall_positions = positions;
}

void BSPResult::set_corridor_positions(const PackedVector2Array& positions) {
    corridor_positions = positions;
}

PackedVector2Array BSPResult::get_floor_positions() const {
    return floor_positions;
}

PackedVector2Array BSPResult::get_wall_positions() const {
    return wall_positions;
}

PackedVector2Array BSPResult::get_corridor_positions() const {
    return corridor_positions;
}

int BSPResult::get_floor_count() const {
    return floor_positions.size();
}

// BSPDungeonGenerator implementation
BSPDungeonGenerator::BSPDungeonGenerator()
    : map_width(64), map_height(64), min_room_size(5), max_room_size(12),
      max_splits(6), room_padding(1), use_seed(false), seed(12345),
      rng(std::random_device{}()) {
}

BSPDungeonGenerator::~BSPDungeonGenerator() {
}

void BSPDungeonGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_map_size", "width", "height"), &BSPDungeonGenerator::set_map_size);
    ClassDB::bind_method(D_METHOD("set_room_size_range", "min_size", "max_size"), &BSPDungeonGenerator::set_room_size_range);
    ClassDB::bind_method(D_METHOD("set_max_splits", "splits"), &BSPDungeonGenerator::set_max_splits);
    ClassDB::bind_method(D_METHOD("set_room_padding", "padding"), &BSPDungeonGenerator::set_room_padding);
    ClassDB::bind_method(D_METHOD("set_use_seed", "enabled"), &BSPDungeonGenerator::set_use_seed);
    ClassDB::bind_method(D_METHOD("set_seed", "seed_value"), &BSPDungeonGenerator::set_seed);
    ClassDB::bind_method(D_METHOD("generate"), &BSPDungeonGenerator::generate);

    ClassDB::bind_method(D_METHOD("get_map_width"), &BSPDungeonGenerator::get_map_width);
    ClassDB::bind_method(D_METHOD("get_map_height"), &BSPDungeonGenerator::get_map_height);
    ClassDB::bind_method(D_METHOD("get_min_room_size"), &BSPDungeonGenerator::get_min_room_size);
    ClassDB::bind_method(D_METHOD("get_max_room_size"), &BSPDungeonGenerator::get_max_room_size);
    ClassDB::bind_method(D_METHOD("get_max_splits"), &BSPDungeonGenerator::get_max_splits);
    ClassDB::bind_method(D_METHOD("get_room_padding"), &BSPDungeonGenerator::get_room_padding);
    ClassDB::bind_method(D_METHOD("get_use_seed"), &BSPDungeonGenerator::get_use_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &BSPDungeonGenerator::get_seed);
}

void BSPDungeonGenerator::set_map_size(int width, int height) {
    map_width = width;
    map_height = height;
}

void BSPDungeonGenerator::set_room_size_range(int min_size, int max_size) {
    min_room_size = min_size;
    max_room_size = max_size;
}

void BSPDungeonGenerator::set_max_splits(int splits) {
    max_splits = splits;
}

void BSPDungeonGenerator::set_room_padding(int padding) {
    room_padding = padding;
}

void BSPDungeonGenerator::set_use_seed(bool enabled) {
    use_seed = enabled;
}

void BSPDungeonGenerator::set_seed(int seed_value) {
    seed = seed_value;
}

int BSPDungeonGenerator::randi_range(int from, int to) {
    if (from > to)
        std::swap(from, to);
    return std::uniform_int_distribution<>(from, to)(rng);
}

void BSPDungeonGenerator::connect_rooms(BSPLeaf* node) {
    if (!node || (!node->left && !node->right))
        return;

    if (node->left && node->right) {
        BSPVec2i a = get_representative_point(node->left);
        BSPVec2i b = get_representative_point(node->right);
        if (!(a == BSPVec2i{0, 0}) && !(b == BSPVec2i{0, 0})) {
            create_corridor(a, b);
        }
    }

    connect_rooms(node->left);
    connect_rooms(node->right);
}

BSPVec2i BSPDungeonGenerator::get_representative_point(BSPLeaf* leaf) {
    if (!leaf)
        return {0, 0};
    if (leaf->has_room)
        return leaf->room.center();
    BSPVec2i left = get_representative_point(leaf->left);
    if (!(left == BSPVec2i{0, 0}))
        return left;
    return get_representative_point(leaf->right);
}

void BSPDungeonGenerator::create_corridor(BSPVec2i a, BSPVec2i b) {
    BSPVec2i pos = a;
    if (std::uniform_real_distribution<>(0, 1)(rng) < 0.5) {
        while (pos.x != b.x) {
            pos.x += (b.x > pos.x ? 1 : -1);
            corridors.push_back(pos);
        }
        while (pos.y != b.y) {
            pos.y += (b.y > pos.y ? 1 : -1);
            corridors.push_back(pos);
        }
    } else {
        while (pos.y != b.y) {
            pos.y += (b.y > pos.y ? 1 : -1);
            corridors.push_back(pos);
        }
        while (pos.x != b.x) {
            pos.x += (b.x > pos.x ? 1 : -1);
            corridors.push_back(pos);
        }
    }
}

Ref<BSPResult> BSPDungeonGenerator::generate() {
    // Initialize RNG
    if (use_seed) {
        rng.seed(seed);
    } else {
        rng.seed(std::random_device{}());
    }

    leaves.clear();
    rooms.clear();
    corridors.clear();

    // BSP partitioning
    BSPLeaf* root = new BSPLeaf({BSPVec2i{0, 0}, BSPVec2i{map_width, map_height}});
    std::vector<BSPLeaf*> queue = {root};
    int split_count = 0;

    while (split_count < max_splits && !queue.empty()) {
        std::vector<BSPLeaf*> next;
        bool did_split = false;
        for (auto* leaf : queue) {
            if (leaf->split(min_room_size, rng)) {
                next.push_back(leaf->left);
                next.push_back(leaf->right);
                did_split = true;
            } else {
                leaves.push_back(leaf);
            }
        }
        if (!did_split) {
            leaves.insert(leaves.end(), next.begin(), next.end());
            break;
        }
        queue = next;
        split_count++;
    }

    if (split_count == max_splits) {
        leaves.insert(leaves.end(), queue.begin(), queue.end());
    }

    // Create rooms in leaves
    for (auto* leaf : leaves) {
        int max_w = leaf->rect.size.x - 2 * room_padding;
        int max_h = leaf->rect.size.y - 2 * room_padding;
        if (max_w < min_room_size || max_h < min_room_size)
            continue;

        int w = randi_range(min_room_size, std::min(max_w, max_room_size));
        int h = randi_range(min_room_size, std::min(max_h, max_room_size));
        int x = randi_range(leaf->rect.position.x + room_padding,
                            leaf->rect.position.x + leaf->rect.size.x - room_padding - w);
        int y = randi_range(leaf->rect.position.y + room_padding,
                            leaf->rect.position.y + leaf->rect.size.y - room_padding - h);

        leaf->room = {{x, y}, {w, h}};
        leaf->has_room = true;
        rooms.push_back(leaf->room);
    }

    // Connect rooms with corridors
    connect_rooms(root);

    // Build result
    Ref<BSPResult> result;
    result.instantiate();

    std::set<BSPVec2i> placed;
    PackedVector2Array floor_positions;
    PackedVector2Array corridor_positions;

    // Add room floors
    for (const auto& room : rooms) {
        for (int y = room.position.y; y < room.position.y + room.size.y; ++y) {
            for (int x = room.position.x; x < room.position.x + room.size.x; ++x) {
                BSPVec2i pos = {x, y};
                placed.insert(pos);
                floor_positions.push_back(Vector2(x, y));
            }
        }
    }

    // Add corridors
    for (const auto& tile : corridors) {
        if (placed.insert(tile).second) {
            corridor_positions.push_back(Vector2(tile.x, tile.y));
            floor_positions.push_back(Vector2(tile.x, tile.y));  // Also add to floors so corridors have floor tiles
        }
    }

    // Generate walls around all placed tiles
    const std::vector<BSPVec2i> offsets = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0},
                                        {1, 0},   {-1, 1}, {0, 1},  {1, 1}};
    std::set<BSPVec2i> wall_positions_set;
    for (const auto& pos : placed) {
        for (const auto& offset : offsets) {
            BSPVec2i neighbor = {pos.x + offset.x, pos.y + offset.y};
            if (!placed.count(neighbor) && !wall_positions_set.count(neighbor)) {
                wall_positions_set.insert(neighbor);
            }
        }
    }

    PackedVector2Array wall_positions;
    for (const auto& pos : wall_positions_set) {
        wall_positions.push_back(Vector2(pos.x, pos.y));
    }

    result->set_floor_positions(floor_positions);
    result->set_corridor_positions(corridor_positions);
    result->set_wall_positions(wall_positions);

    delete root;

    return result;
}
