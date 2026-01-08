#include "hybrid_godot.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// --- HybridResult ---

HybridResult::HybridResult() {
    grid_width = 0;
    grid_height = 0;
    tile_w = 0;
    tile_h = 0;
}

void HybridResult::set_data(const std::vector<RoomObj>& p_rooms, 
                           const std::vector<Link>& p_links,
                           const std::vector<Point>& p_floors,
                           const std::vector<Point>& p_walls,
                           const GenSettings& p_cfg) {
    
    // Convert Rooms
    rooms.clear();
    for (const auto& r : p_rooms) {
        Dictionary d;
        d["id"] = r.id;
        d["x"] = r.x;
        d["y"] = r.y;
        d["w"] = r.w;
        d["h"] = r.h;
        d["shape"] = (r.shape == Shape::Rect) ? "rect" : "circle";
        d["is_main"] = r.isMain;
        rooms.append(d);
    }

    // Convert Links
    links.clear();
    for (const auto& l : p_links) {
        Dictionary d;
        d["u"] = l.u;
        d["v"] = l.v;
        d["is_mst"] = l.isMST;
        links.append(d);
    }

    // Convert Floors
    floors.resize(p_floors.size());
    for (size_t i = 0; i < p_floors.size(); ++i) {
        floors[i] = Vector2(p_floors[i].x, p_floors[i].y);
    }

    // Convert Walls
    walls.resize(p_walls.size());
    for (size_t i = 0; i < p_walls.size(); ++i) {
        walls[i] = Vector2(p_walls[i].x, p_walls[i].y);
    }

    // Config
    grid_width = p_cfg.gridWidth;
    grid_height = p_cfg.gridHeight;
    tile_w = p_cfg.tileW;
    tile_h = p_cfg.tileH;
}

Array HybridResult::get_rooms() const { return rooms; }
Array HybridResult::get_links() const { return links; }
PackedVector2Array HybridResult::get_floors() const { return floors; }
PackedVector2Array HybridResult::get_walls() const { return walls; }

int HybridResult::get_total_tiles() const { 
    return floors.size() + walls.size(); 
}

int HybridResult::get_grid_width() const { return grid_width; }
int HybridResult::get_grid_height() const { return grid_height; }
int HybridResult::get_tile_w() const { return tile_w; }
int HybridResult::get_tile_h() const { return tile_h; }

void HybridResult::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_rooms"), &HybridResult::get_rooms);
    ClassDB::bind_method(D_METHOD("get_links"), &HybridResult::get_links);
    ClassDB::bind_method(D_METHOD("get_floors"), &HybridResult::get_floors);
    ClassDB::bind_method(D_METHOD("get_walls"), &HybridResult::get_walls);
    ClassDB::bind_method(D_METHOD("get_total_tiles"), &HybridResult::get_total_tiles);
    ClassDB::bind_method(D_METHOD("get_grid_width"), &HybridResult::get_grid_width);
    ClassDB::bind_method(D_METHOD("get_grid_height"), &HybridResult::get_grid_height);
    ClassDB::bind_method(D_METHOD("get_tile_w"), &HybridResult::get_tile_w);
    ClassDB::bind_method(D_METHOD("get_tile_h"), &HybridResult::get_tile_h);
}

// --- HybridDungeonGenerator ---

HybridDungeonGenerator::HybridDungeonGenerator() {
    // Default settings
    settings.roomCount = 150;
    settings.spreadRadius = 50.0f;
    settings.walkerCount = 400;
    settings.gridWidth = 200;
    settings.gridHeight = 150;
    settings.tileW = 4;
    settings.tileH = 4;
    settings.seed = 0;
}

void HybridDungeonGenerator::set_room_count(int count) { settings.roomCount = count; }
int HybridDungeonGenerator::get_room_count() const { return settings.roomCount; }

void HybridDungeonGenerator::set_spread_radius(float radius) { settings.spreadRadius = radius; }
float HybridDungeonGenerator::get_spread_radius() const { return settings.spreadRadius; }

void HybridDungeonGenerator::set_walker_count(int count) { settings.walkerCount = count; }
int HybridDungeonGenerator::get_walker_count() const { return settings.walkerCount; }

void HybridDungeonGenerator::set_grid_size(int width, int height) { 
    settings.gridWidth = width; 
    settings.gridHeight = height; 
}
void HybridDungeonGenerator::set_grid_width(int width) { settings.gridWidth = width; }
void HybridDungeonGenerator::set_grid_height(int height) { settings.gridHeight = height; }

int HybridDungeonGenerator::get_grid_width() const { return settings.gridWidth; }
int HybridDungeonGenerator::get_grid_height() const { return settings.gridHeight; }

void HybridDungeonGenerator::set_tile_size(int w, int h) { 
    settings.tileW = w; 
    settings.tileH = h; 
}
void HybridDungeonGenerator::set_tile_w(int w) { settings.tileW = w; }
void HybridDungeonGenerator::set_tile_h(int h) { settings.tileH = h; }

int HybridDungeonGenerator::get_tile_w() const { return settings.tileW; }
int HybridDungeonGenerator::get_tile_h() const { return settings.tileH; }

void HybridDungeonGenerator::set_seed(int seed) { settings.seed = seed; }
int HybridDungeonGenerator::get_seed() const { return settings.seed; }

Ref<HybridResult> HybridDungeonGenerator::generate() {
    DungeonBuilder builder;
    builder.init(settings);

    // Run until complete
    int max_steps = 100000; 
    while (!builder.isComplete() && max_steps > 0) {
        builder.step();
        max_steps--;
    }

    Ref<HybridResult> res;
    res.instantiate();
    
    res->set_data(
        builder.getRooms(),
        builder.getLinks(),
        builder.getFloors(),
        builder.getWalls(),
        settings 
    );

    return res;
}

void HybridDungeonGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate"), &HybridDungeonGenerator::generate);
    
    ClassDB::bind_method(D_METHOD("set_room_count", "count"), &HybridDungeonGenerator::set_room_count);
    ClassDB::bind_method(D_METHOD("get_room_count"), &HybridDungeonGenerator::get_room_count);
    
    ClassDB::bind_method(D_METHOD("set_spread_radius", "radius"), &HybridDungeonGenerator::set_spread_radius);
    ClassDB::bind_method(D_METHOD("get_spread_radius"), &HybridDungeonGenerator::get_spread_radius);
    
    ClassDB::bind_method(D_METHOD("set_walker_count", "count"), &HybridDungeonGenerator::set_walker_count);
    ClassDB::bind_method(D_METHOD("get_walker_count"), &HybridDungeonGenerator::get_walker_count);
    
    ClassDB::bind_method(D_METHOD("set_grid_size", "width", "height"), &HybridDungeonGenerator::set_grid_size);
    ClassDB::bind_method(D_METHOD("set_grid_width", "width"), &HybridDungeonGenerator::set_grid_width);
    ClassDB::bind_method(D_METHOD("set_grid_height", "height"), &HybridDungeonGenerator::set_grid_height);
    ClassDB::bind_method(D_METHOD("get_grid_width"), &HybridDungeonGenerator::get_grid_width);
    ClassDB::bind_method(D_METHOD("get_grid_height"), &HybridDungeonGenerator::get_grid_height);

    ClassDB::bind_method(D_METHOD("set_tile_size", "w", "h"), &HybridDungeonGenerator::set_tile_size);
    ClassDB::bind_method(D_METHOD("set_tile_w", "w"), &HybridDungeonGenerator::set_tile_w);
    ClassDB::bind_method(D_METHOD("set_tile_h", "h"), &HybridDungeonGenerator::set_tile_h);
    ClassDB::bind_method(D_METHOD("get_tile_w"), &HybridDungeonGenerator::get_tile_w);
    ClassDB::bind_method(D_METHOD("get_tile_h"), &HybridDungeonGenerator::get_tile_h);
    
    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &HybridDungeonGenerator::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &HybridDungeonGenerator::get_seed);

    // Properties
    ADD_PROPERTY(PropertyInfo(Variant::INT, "room_count"), "set_room_count", "get_room_count");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spread_radius"), "set_spread_radius", "get_spread_radius");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "walker_count"), "set_walker_count", "get_walker_count");
    
    ADD_GROUP("Grid", "grid_");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "grid_width"), "set_grid_width", "get_grid_width");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "grid_height"), "set_grid_height", "get_grid_height");
    
    ADD_GROUP("Tile", "tile_");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_w"), "set_tile_w", "get_tile_w");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_h"), "set_tile_h", "get_tile_h");

    ADD_GROUP("", "");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
}