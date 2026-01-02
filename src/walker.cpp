    // walker.cpp - Walker dungeon generation implementation

#include "walker.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>
#include <cmath>

using namespace godot;

// ============================================================================
// WalkerResult Implementation
// ============================================================================

WalkerResult::WalkerResult() : map_width(0), map_height(0) {}

WalkerResult::~WalkerResult() {}

void WalkerResult::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_floor_positions"), &WalkerResult::get_floor_positions);
    ClassDB::bind_method(D_METHOD("get_wall_positions"), &WalkerResult::get_wall_positions);
    ClassDB::bind_method(D_METHOD("get_map_width"), &WalkerResult::get_map_width);
    ClassDB::bind_method(D_METHOD("get_map_height"), &WalkerResult::get_map_height);
    ClassDB::bind_method(D_METHOD("get_tilemap_positions_with_atlas", "tilemap_layer", "atlas_coords", "source_id"),
                        &WalkerResult::get_tilemap_positions_with_atlas, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("get_statistics"), &WalkerResult::get_statistics);
}

PackedVector2Array WalkerResult::get_tilemap_positions_with_atlas(Object* tilemap_layer, Vector2i atlas_coords, int source_id) const {
    PackedVector2Array result;

    if (!tilemap_layer) {
        UtilityFunctions::push_error("WalkerResult: tilemap_layer is null");
        return result;
    }

    // Iterate through all floor positions and check what's actually on the tilemap
    for (int i = 0; i < floor_positions.size(); i++) {
        Vector2 pos = floor_positions[i];
        Vector2i cell_pos = Vector2i(int(pos.x), int(pos.y));

        // Call get_cell_atlas_coords on the TileMapLayer
        Vector2i cell_atlas = tilemap_layer->call("get_cell_atlas_coords", cell_pos);
        int cell_source = (int)tilemap_layer->call("get_cell_source_id", cell_pos);

        // Check if it matches what we're looking for
        if (cell_atlas == atlas_coords && cell_source == source_id) {
            result.append(pos);
        }
    }

    return result;
}

Dictionary WalkerResult::get_statistics() const {
    Dictionary stats;
    stats["floor_count"] = floor_positions.size();
    stats["wall_count"] = wall_positions.size();
    stats["map_width"] = map_width;
    stats["map_height"] = map_height;
    return stats;
}

void WalkerResult::_set_result_data(PackedVector2Array floors, PackedVector2Array walls, int width, int height) {
    floor_positions = floors;
    wall_positions = walls;
    map_width = width;
    map_height = height;
}

// ============================================================================
// WalkerDungeonGenerator Implementation
// ============================================================================

WalkerDungeonGenerator::WalkerDungeonGenerator()
    : allow_overlap(false), min_hall(3), max_hall(6), room_dim(5),
      total_floor_count(200), seed(0), use_seed(false), map_size(0, 0) {
    rng.seed(std::random_device{}());
}

WalkerDungeonGenerator::~WalkerDungeonGenerator() {}

void WalkerDungeonGenerator::_bind_methods() {
    // Configuration methods
    ClassDB::bind_method(D_METHOD("set_allow_overlap", "allow_overlap"), &WalkerDungeonGenerator::set_allow_overlap);
    ClassDB::bind_method(D_METHOD("get_allow_overlap"), &WalkerDungeonGenerator::get_allow_overlap);

    ClassDB::bind_method(D_METHOD("set_min_hall", "min_hall"), &WalkerDungeonGenerator::set_min_hall);
    ClassDB::bind_method(D_METHOD("get_min_hall"), &WalkerDungeonGenerator::get_min_hall);

    ClassDB::bind_method(D_METHOD("set_max_hall", "max_hall"), &WalkerDungeonGenerator::set_max_hall);
    ClassDB::bind_method(D_METHOD("get_max_hall"), &WalkerDungeonGenerator::get_max_hall);

    ClassDB::bind_method(D_METHOD("set_room_dim", "room_dim"), &WalkerDungeonGenerator::set_room_dim);
    ClassDB::bind_method(D_METHOD("get_room_dim"), &WalkerDungeonGenerator::get_room_dim);

    ClassDB::bind_method(D_METHOD("set_total_floor_count", "total_floor_count"), &WalkerDungeonGenerator::set_total_floor_count);
    ClassDB::bind_method(D_METHOD("get_total_floor_count"), &WalkerDungeonGenerator::get_total_floor_count);

    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &WalkerDungeonGenerator::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &WalkerDungeonGenerator::get_seed);

    ClassDB::bind_method(D_METHOD("set_use_seed", "use_seed"), &WalkerDungeonGenerator::set_use_seed);
    ClassDB::bind_method(D_METHOD("get_use_seed"), &WalkerDungeonGenerator::get_use_seed);

    // Generation method
    ClassDB::bind_method(D_METHOD("generate"), &WalkerDungeonGenerator::generate);

    // Properties for inspector
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_overlap"), "set_allow_overlap", "get_allow_overlap");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "min_hall", PROPERTY_HINT_RANGE, "1,20,1"), "set_min_hall", "get_min_hall");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_hall", PROPERTY_HINT_RANGE, "1,30,1"), "set_max_hall", "get_max_hall");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "room_dim", PROPERTY_HINT_RANGE, "2,15,1"), "set_room_dim", "get_room_dim");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "total_floor_count", PROPERTY_HINT_RANGE, "50,5000,10"), "set_total_floor_count", "get_total_floor_count");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_seed"), "set_use_seed", "get_use_seed");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
}

void WalkerDungeonGenerator::set_allow_overlap(bool p_allow_overlap) {
    allow_overlap = p_allow_overlap;
}

void WalkerDungeonGenerator::set_min_hall(int p_min_hall) {
    min_hall = std::max(1, p_min_hall);
}

void WalkerDungeonGenerator::set_max_hall(int p_max_hall) {
    max_hall = std::max(min_hall, p_max_hall);
}

void WalkerDungeonGenerator::set_room_dim(int p_room_dim) {
    room_dim = std::max(2, p_room_dim);
}

void WalkerDungeonGenerator::set_total_floor_count(int p_total_floor_count) {
    total_floor_count = std::max(50, p_total_floor_count);
}

void WalkerDungeonGenerator::set_seed(int p_seed) {
    seed = p_seed;
}

void WalkerDungeonGenerator::set_use_seed(bool p_use_seed) {
    use_seed = p_use_seed;
}

void WalkerDungeonGenerator::calculate_map_size() {
    float fill_ratio = allow_overlap ? 0.50f : 0.65f;
    int total_tiles_needed = (int)(total_floor_count / fill_ratio);
    int side_length = (int)std::sqrt(total_tiles_needed) + (allow_overlap ? 15 : 10);
    side_length = std::max(side_length, (allow_overlap ? 30 : 20));
    map_size = Vector2i(side_length, side_length);
}

Vector2i WalkerDungeonGenerator::random_direction(Vector2i current) {
    static const std::vector<Vector2i> directions = {
        Vector2i(0, -1), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(1, 0)
    };

    Vector2i dir;
    int tries = 0;
    do {
        dir = directions[randi_range(0, directions.size() - 1)];
        tries++;
    } while (allow_overlap && current.x != 0 && current.y != 0 &&
             dir.x == -current.x && dir.y == -current.y && tries < 10);

    return dir;
}

int WalkerDungeonGenerator::randi_range(int from, int to) {
    if (from > to) {
        std::swap(from, to);
    }
    return std::uniform_int_distribution<>(from, to)(rng);
}

bool WalkerDungeonGenerator::room_overlaps_existing(Vector2i center, int width, int height) {
    if (allow_overlap) {
        return false;
    }

    int margin = 1;
    for (const auto& existing : rooms) {
        if (center.x - width / 2 - margin < existing.center.x + existing.width / 2 &&
            center.x + width / 2 + margin > existing.center.x - existing.width / 2 &&
            center.y - height / 2 - margin < existing.center.y + existing.height / 2 &&
            center.y + height / 2 + margin > existing.center.y - existing.height / 2) {
            return true;
        }
    }
    return false;
}

void WalkerDungeonGenerator::spawn_walker() {
    Vector2i start;
    if (floor_tiles.empty()) {
        start = Vector2i(map_size.x / 2, map_size.y / 2);
        floor_tiles.insert(start);
    } else {
        auto it = floor_tiles.begin();
        std::advance(it, randi_range(0, floor_tiles.size() - 1));
        start = *it;
    }

    walkers.push_back({start, random_direction(), Vector2i(0, 0)});
}

void WalkerDungeonGenerator::place_organic_room(Vector2i center, int w, int h) {
    float rx = std::max(1.f, w / 2.f);
    float ry = std::max(1.f, h / 2.f);

    for (int y = -(int)std::ceil(ry); y <= (int)std::ceil(ry); ++y) {
        for (int x = -(int)std::ceil(rx); x <= (int)std::ceil(rx); ++x) {
            float nx = x / rx;
            float ny = y / ry;
            if (nx * nx + ny * ny <= 1.0f &&
                randi_range(0, 100) > (int)((1.0f - std::sqrt(nx * nx + ny * ny)) * 20)) {
                Vector2i pos = center + Vector2i(x, y);
                if (pos.x > 0 && pos.x < map_size.x - 1 &&
                    pos.y > 0 && pos.y < map_size.y - 1) {
                    floor_tiles.insert(pos);
                }
            }
        }
    }
}

bool WalkerDungeonGenerator::try_place_room(Vector2i center) {
    for (int i = 0; i < 3; ++i) {
        int d = std::max(3, room_dim - i * 2);
        int rx = randi_range(1, d / 2 + 1);
        int ry = randi_range(1, d / 2 + 1);

        if (allow_overlap && randi_range(0, 100) < 70) {
            place_organic_room(center, rx * 2, ry * 2);
            return true;
        }

        if (center.x - rx <= 0 || center.x + rx >= map_size.x - 1 ||
            center.y - ry <= 0 || center.y + ry >= map_size.y - 1) {
            continue;
        }

        if (!allow_overlap && room_overlaps_existing(center, rx, ry)) {
            continue;
        }

        for (int x = -rx; x <= rx; ++x) {
            for (int y = -ry; y <= ry; ++y) {
                floor_tiles.insert(center + Vector2i(x, y));
            }
        }

        if (!allow_overlap) {
            rooms.push_back({center, rx, ry});
        }
        return true;
    }
    return false;
}

void WalkerDungeonGenerator::simulate_walkers() {
    Vector2i start(map_size.x / 2, map_size.y / 2);
    floor_tiles.insert(start);

    if (!allow_overlap) {
        try_place_room(start);
    } else {
        for (int i = 0; i < 3; i++) {
            spawn_walker();
        }
    }

    int attempts = 0;
    const int max_attempts = allow_overlap ? 150000 : 50000;

    while (floor_tiles.size() < (size_t)total_floor_count && attempts++ < max_attempts) {
        if (allow_overlap) {
            std::vector<Walker> next_gen;
            if (walkers.empty()) {
                spawn_walker();
            }

            for (auto& w : walkers) {
                Vector2i new_pos = w.position + w.direction;
                if (new_pos.x > 0 && new_pos.x < map_size.x - 1 &&
                    new_pos.y > 0 && new_pos.y < map_size.y - 1) {
                    floor_tiles.insert(new_pos);
                    w.position = new_pos;
                    w.last_direction = w.direction;

                    // Widen corridors occasionally
                    if (randi_range(0, 100) < 10) {
                        Vector2i perp(w.direction.y, w.direction.x);
                        floor_tiles.insert(new_pos + perp);
                        floor_tiles.insert(new_pos - perp);
                    }

                    // Change direction
                    if (randi_range(0, 100) < 15) {
                        w.direction = random_direction(w.last_direction);
                    }

                    // Spawn new walker
                    if (randi_range(0, 100) < 10 && walkers.size() < 50) {
                        next_gen.push_back({w.position, random_direction(w.last_direction), w.last_direction});
                    }

                    // Place organic room
                    if (randi_range(0, 100) < 7) {
                        place_organic_room(w.position,
                                         randi_range(room_dim / 2, room_dim),
                                         randi_range(room_dim / 2, room_dim));
                    }

                    next_gen.push_back(w);
                }
            }
            walkers = next_gen;
        } else {
            // Non-overlap mode: single walker with rooms
            Vector2i cur_pos = start;
            if (!rooms.empty() && randi_range(0, 100) < 70) {
                cur_pos = rooms[randi_range(0, rooms.size() - 1)].center;
            } else if (!floor_tiles.empty()) {
                auto it = floor_tiles.begin();
                std::advance(it, randi_range(0, floor_tiles.size() - 1));
                cur_pos = *it;
            }

            Vector2i dir = random_direction();
            int len = randi_range(min_hall, max_hall);
            for (int i = 0; i < len; ++i) {
                Vector2i next = cur_pos + dir;
                if (next.x > 1 && next.x < map_size.x - 2 &&
                    next.y > 1 && next.y < map_size.y - 2) {
                    floor_tiles.insert(next);
                    cur_pos = next;
                } else {
                    dir = random_direction(dir);
                    next = cur_pos + dir;
                    if (next.x > 1 && next.x < map_size.x - 2 &&
                        next.y > 1 && next.y < map_size.y - 2) {
                        floor_tiles.insert(next);
                        cur_pos = next;
                    } else {
                        break;
                    }
                }
            }
            try_place_room(cur_pos);
        }
    }
}

void WalkerDungeonGenerator::generate_walls() {
    walls.clear();

    // Check 4 directions (N/S/E/W) around each floor tile
    std::vector<Vector2i> neighbors = {
        Vector2i(1, 0), Vector2i(-1, 0),
        Vector2i(0, 1), Vector2i(0, -1)
    };

    for (const Vector2i& tile : floor_tiles) {
        for (const Vector2i& offset : neighbors) {
            Vector2i n = tile + offset;
            if (floor_tiles.find(n) == floor_tiles.end()) {
                walls.insert(n);
            }
        }
    }
}

Ref<WalkerResult> WalkerDungeonGenerator::generate() {
    // Initialize RNG
    if (use_seed) {
        rng.seed(seed);
    } else {
        rng.seed(std::random_device{}());
    }

    // Clear previous state
    walkers.clear();
    floor_tiles.clear();
    rooms.clear();
    walls.clear();

    // Calculate map size based on parameters
    calculate_map_size();

    // Generate dungeon
    simulate_walkers();
    generate_walls();

    // Convert to Godot arrays
    PackedVector2Array floor_array;
    PackedVector2Array wall_array;

    floor_array.resize(floor_tiles.size());
    int idx = 0;
    for (const Vector2i& pos : floor_tiles) {
        floor_array[idx++] = Vector2(pos.x, pos.y);
    }

    wall_array.resize(walls.size());
    idx = 0;
    for (const Vector2i& pos : walls) {
        wall_array[idx++] = Vector2(pos.x, pos.y);
    }

    // Create result
    Ref<WalkerResult> result;
    result.instantiate();
    result->_set_result_data(floor_array, wall_array, map_size.x, map_size.y);

    return result;
}
