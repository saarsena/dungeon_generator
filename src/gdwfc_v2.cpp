#include "gdwfc_v2.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <tuple>

#include "../tiling-wfc/include/tiling_wfc.hpp"
#include "../tiling-wfc/include/utils/array2D.hpp"

// ============================================================================
// WFCResult Implementation
// ============================================================================

WFCResult::WFCResult() :
    wfc_width(0), wfc_height(0),
    expanded_width(0), expanded_height(0),
    has_stamps(false), stamp_size(0),
    success(false) {
}

WFCResult::~WFCResult() {
}

void WFCResult::_bind_methods() {
    // Result queries
    ClassDB::bind_method(D_METHOD("is_success"), &WFCResult::is_success);
    ClassDB::bind_method(D_METHOD("get_failure_reason"), &WFCResult::get_failure_reason);
    ClassDB::bind_method(D_METHOD("get_failure_position"), &WFCResult::get_failure_position);

    // Dimensions
    ClassDB::bind_method(D_METHOD("get_wfc_width"), &WFCResult::get_wfc_width);
    ClassDB::bind_method(D_METHOD("get_wfc_height"), &WFCResult::get_wfc_height);
    ClassDB::bind_method(D_METHOD("get_expanded_width"), &WFCResult::get_expanded_width);
    ClassDB::bind_method(D_METHOD("get_expanded_height"), &WFCResult::get_expanded_height);

    // Tile access
    ClassDB::bind_method(D_METHOD("get_wfc_tile_at", "x", "y"), &WFCResult::get_wfc_tile_at);
    ClassDB::bind_method(D_METHOD("get_expanded_tile_at", "x", "y"), &WFCResult::get_expanded_tile_at);
    ClassDB::bind_method(D_METHOD("get_wfc_tiles"), &WFCResult::get_wfc_tiles);
    ClassDB::bind_method(D_METHOD("get_expanded_tiles"), &WFCResult::get_expanded_tiles);

    // Helper methods
    ClassDB::bind_method(D_METHOD("get_floor_positions", "floor_value"), &WFCResult::get_floor_positions, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("get_wall_positions", "wall_value"), &WFCResult::get_wall_positions, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("get_tile_distribution"), &WFCResult::get_tile_distribution);

    // Internal
    ClassDB::bind_method(D_METHOD("_set_wfc_data", "tiles", "width", "height"), &WFCResult::_set_wfc_data);
    ClassDB::bind_method(D_METHOD("_set_expanded_data", "tiles", "width", "height", "stamp_size"), &WFCResult::_set_expanded_data);
    ClassDB::bind_method(D_METHOD("_set_failure", "reason", "position"), &WFCResult::_set_failure);
}

int WFCResult::get_wfc_tile_at(int x, int y) const {
    if (x < 0 || x >= wfc_width || y < 0 || y >= wfc_height) {
        return -1;
    }
    return wfc_tiles[y * wfc_width + x];
}

int WFCResult::get_expanded_tile_at(int x, int y) const {
    if (!has_stamps) {
        return get_wfc_tile_at(x, y);
    }
    if (x < 0 || x >= expanded_width || y < 0 || y >= expanded_height) {
        return -1;
    }
    return expanded_tiles[y * expanded_width + x];
}

PackedVector2Array WFCResult::get_floor_positions(int floor_tile_value) const {
    PackedVector2Array positions;
    const PackedInt32Array& tiles = has_stamps ? expanded_tiles : wfc_tiles;
    int w = has_stamps ? expanded_width : wfc_width;
    int h = has_stamps ? expanded_height : wfc_height;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int tile = tiles[y * w + x];
            if (tile == floor_tile_value) {
                positions.append(Vector2(x, y));
            }
        }
    }
    return positions;
}

PackedVector2Array WFCResult::get_wall_positions(int wall_tile_value) const {
    PackedVector2Array positions;
    const PackedInt32Array& tiles = has_stamps ? expanded_tiles : wfc_tiles;
    int w = has_stamps ? expanded_width : wfc_width;
    int h = has_stamps ? expanded_height : wfc_height;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int tile = tiles[y * w + x];
            if (tile == wall_tile_value) {
                positions.append(Vector2(x, y));
            }
        }
    }
    return positions;
}

Dictionary WFCResult::get_tile_distribution() const {
    Dictionary dist;
    const PackedInt32Array& tiles = has_stamps ? expanded_tiles : wfc_tiles;

    for (int i = 0; i < tiles.size(); i++) {
        int tile_id = tiles[i];
        if (dist.has(tile_id)) {
            dist[tile_id] = (int)dist[tile_id] + 1;
        } else {
            dist[tile_id] = 1;
        }
    }
    return dist;
}

void WFCResult::_set_wfc_data(PackedInt32Array tiles, int width, int height) {
    wfc_tiles = tiles;
    wfc_width = width;
    wfc_height = height;
    success = true;
}

void WFCResult::_set_expanded_data(PackedInt32Array tiles, int width, int height, int p_stamp_size) {
    expanded_tiles = tiles;
    expanded_width = width;
    expanded_height = height;
    has_stamps = true;
    stamp_size = p_stamp_size;
}

void WFCResult::_set_failure(String reason, Vector2i position) {
    success = false;
    failure_reason = reason;
    failure_position = position;
}

// ============================================================================
// WFCConfiguration Implementation
// ============================================================================

WFCConfiguration::WFCConfiguration() :
    use_connection_system(false),
    rules_auto_generated(false),
    stamp_size(0) {
}

WFCConfiguration::~WFCConfiguration() {
}

void WFCConfiguration::_bind_methods() {
    // Configuration modes
    ClassDB::bind_method(D_METHOD("enable_connection_system", "enabled"), &WFCConfiguration::enable_connection_system, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("set_stamp_size", "size"), &WFCConfiguration::set_stamp_size);

    // Add tiles
    ClassDB::bind_method(D_METHOD("add_tile", "tile_id", "tile_data", "tile_size", "symmetry", "weight"),
                        &WFCConfiguration::add_tile);
    ClassDB::bind_method(D_METHOD("add_connected_tile", "tile_id", "connections", "weight", "symmetry"),
                        &WFCConfiguration::add_connected_tile, DEFVAL(1.0), DEFVAL(SYMMETRY_X));
    ClassDB::bind_method(D_METHOD("set_tile_stamp", "tile_id", "stamp_pattern", "stamp_width", "stamp_height"),
                        &WFCConfiguration::set_tile_stamp);

    // Neighbor rules
    ClassDB::bind_method(D_METHOD("add_neighbor_rule", "tile1_id", "orientation1", "tile2_id", "orientation2"),
                        &WFCConfiguration::add_neighbor_rule);
    ClassDB::bind_method(D_METHOD("auto_generate_neighbor_rules"), &WFCConfiguration::auto_generate_neighbor_rules);
    ClassDB::bind_method(D_METHOD("add_border_all_tile", "tile_id"), &WFCConfiguration::add_border_all_tile);

    // Validation
    ClassDB::bind_method(D_METHOD("validate_rules"), &WFCConfiguration::validate_rules);
    ClassDB::bind_method(D_METHOD("get_tile_info", "tile_id"), &WFCConfiguration::get_tile_info);
    ClassDB::bind_method(D_METHOD("get_tile_count"), &WFCConfiguration::get_tile_count);
    ClassDB::bind_method(D_METHOD("get_rule_count"), &WFCConfiguration::get_rule_count);

    // Clear
    ClassDB::bind_method(D_METHOD("clear"), &WFCConfiguration::clear);
    ClassDB::bind_method(D_METHOD("clear_rules_only"), &WFCConfiguration::clear_rules_only);

    // Enums
    BIND_ENUM_CONSTANT(SYMMETRY_X);
    BIND_ENUM_CONSTANT(SYMMETRY_I);
    BIND_ENUM_CONSTANT(SYMMETRY_BACKSLASH);
    BIND_ENUM_CONSTANT(SYMMETRY_T);
    BIND_ENUM_CONSTANT(SYMMETRY_L);
    BIND_ENUM_CONSTANT(SYMMETRY_P);
}

void WFCConfiguration::enable_connection_system(bool enabled) {
    use_connection_system = enabled;
}

void WFCConfiguration::set_stamp_size(int size) {
    stamp_size = size;
}

void WFCConfiguration::add_tile(int tile_id, PackedInt32Array tile_data, int tile_size,
                                SymmetryType symmetry, float weight) {
    TileDefinition tile;
    tile.tile_id = tile_id;
    tile.tile_data = tile_data;
    tile.tile_size = tile_size;
    tile.symmetry = symmetry;
    tile.weight = weight;
    tile.has_connections = false;
    tile.borders_all = false;
    tile.has_stamp = false;
    tiles.push_back(tile);
}

void WFCConfiguration::add_connected_tile(int tile_id, Dictionary connections, float weight,
                                          SymmetryType symmetry) {
    TileDefinition tile;
    tile.tile_id = tile_id;
    tile.tile_size = 1;
    tile.symmetry = symmetry;
    tile.weight = weight;

    // Parse connections dictionary
    tile.has_connections = true;
    tile.connect_left = connections.get("left", false);
    tile.connect_up = connections.get("up", false);
    tile.connect_right = connections.get("right", false);
    tile.connect_down = connections.get("down", false);
    tile.borders_all = connections.get("borders_all", false);

    // Create simple tile_data (single value)
    tile.tile_data.append(tile_id);

    tile.has_stamp = false;
    tiles.push_back(tile);
}

void WFCConfiguration::set_tile_stamp(int tile_id, PackedInt32Array stamp_pattern,
                                      int stamp_width, int stamp_height) {
    // Find existing tile
    for (size_t i = 0; i < tiles.size(); i++) {
        if (tiles[i].tile_id == tile_id) {
            tiles[i].has_stamp = true;
            tiles[i].stamp_pattern = stamp_pattern;
            tiles[i].stamp_width = stamp_width;
            tiles[i].stamp_height = stamp_height;
            return;
        }
    }
    UtilityFunctions::push_error("Tile ", tile_id, " not found when setting stamp");
}

void WFCConfiguration::add_neighbor_rule(int tile1_id, int orientation1,
                                         int tile2_id, int orientation2) {
    NeighborRule rule;
    rule.tile1_id = tile1_id;
    rule.orientation1 = orientation1;
    rule.tile2_id = tile2_id;
    rule.orientation2 = orientation2;
    neighbor_rules.push_back(rule);
}

void WFCConfiguration::auto_generate_neighbor_rules() {
    if (!use_connection_system) {
        UtilityFunctions::push_warning("auto_generate_neighbor_rules called but connection system not enabled");
        return;
    }

    clear_rules_only();

    // Generate rules based on edge connections
    for (size_t i = 0; i < tiles.size(); i++) {
        for (size_t j = 0; j < tiles.size(); j++) {
            const TileDefinition& tile1 = tiles[i];
            const TileDefinition& tile2 = tiles[j];

            if (!tile1.has_connections || !tile2.has_connections) {
                continue;
            }

            // Special case: borders_all tiles
            if (tile1.borders_all || tile2.borders_all) {
                add_neighbor_rule(tile1.tile_id, 0, tile2.tile_id, 0);
                continue;
            }

            // Horizontal adjacency: tile1's RIGHT must match tile2's LEFT
            if (tile1.connect_right == tile2.connect_left) {
                add_neighbor_rule(tile1.tile_id, 0, tile2.tile_id, 0);
            }
        }
    }

    rules_auto_generated = true;
    UtilityFunctions::print("Auto-generated ", (int)neighbor_rules.size(), " neighbor rules");
}

void WFCConfiguration::add_border_all_tile(int tile_id) {
    for (size_t i = 0; i < tiles.size(); i++) {
        if (tiles[i].tile_id == tile_id) {
            tiles[i].borders_all = true;
            return;
        }
    }
}

bool WFCConfiguration::validate_rules() const {
    // Check for tiles without any neighbor rules
    std::map<int, int> tile_rule_count;

    for (const auto& rule : neighbor_rules) {
        tile_rule_count[rule.tile1_id]++;
        tile_rule_count[rule.tile2_id]++;
    }

    bool valid = true;
    for (const auto& tile : tiles) {
        if (tile_rule_count[tile.tile_id] == 0) {
            UtilityFunctions::push_warning("Tile ", tile.tile_id, " has no neighbor rules");
            valid = false;
        }
    }

    return valid;
}

Dictionary WFCConfiguration::get_tile_info(int tile_id) const {
    for (const auto& tile : tiles) {
        if (tile.tile_id == tile_id) {
            Dictionary info;
            info["tile_id"] = tile.tile_id;
            info["weight"] = tile.weight;
            info["symmetry"] = tile.symmetry;
            info["has_connections"] = tile.has_connections;
            if (tile.has_connections) {
                info["left"] = tile.connect_left;
                info["up"] = tile.connect_up;
                info["right"] = tile.connect_right;
                info["down"] = tile.connect_down;
                info["borders_all"] = tile.borders_all;
            }
            info["has_stamp"] = tile.has_stamp;
            return info;
        }
    }
    return Dictionary();
}

void WFCConfiguration::clear() {
    tiles.clear();
    neighbor_rules.clear();
    rules_auto_generated = false;
}

void WFCConfiguration::clear_rules_only() {
    neighbor_rules.clear();
    rules_auto_generated = false;
}

// ============================================================================
// GDTilingWFCv2 Implementation
// ============================================================================

GDTilingWFCv2::GDTilingWFCv2() :
    width(10), height(10), seed(0), periodic(false), debug_mode(false) {
    config.instantiate();
}

GDTilingWFCv2::GDTilingWFCv2(Ref<WFCConfiguration> p_config) :
    width(10), height(10), seed(0), periodic(false), debug_mode(false) {
    config = p_config;
}

GDTilingWFCv2::~GDTilingWFCv2() {
}

void GDTilingWFCv2::_bind_methods() {
    // Basic settings
    ClassDB::bind_method(D_METHOD("set_size", "width", "height"), &GDTilingWFCv2::set_size);
    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &GDTilingWFCv2::set_seed);
    ClassDB::bind_method(D_METHOD("set_periodic", "periodic"), &GDTilingWFCv2::set_periodic);
    ClassDB::bind_method(D_METHOD("set_configuration", "config"), &GDTilingWFCv2::set_configuration);
    ClassDB::bind_method(D_METHOD("enable_debug", "enabled"), &GDTilingWFCv2::enable_debug);

    // Quick setup
    ClassDB::bind_method(D_METHOD("use_connection_system"), &GDTilingWFCv2::use_connection_system);
    ClassDB::bind_method(D_METHOD("add_connected_tile", "tile_id", "connections", "weight"),
                        &GDTilingWFCv2::add_connected_tile, DEFVAL(1.0));
    ClassDB::bind_method(D_METHOD("set_tile_stamp", "tile_id", "stamp_pattern", "stamp_width", "stamp_height"),
                        &GDTilingWFCv2::set_tile_stamp);
    ClassDB::bind_method(D_METHOD("auto_generate_rules"), &GDTilingWFCv2::auto_generate_rules);

    // Traditional
    ClassDB::bind_method(D_METHOD("add_tile", "tile_id", "tile_data", "tile_size", "symmetry", "weight"),
                        &GDTilingWFCv2::add_tile);
    ClassDB::bind_method(D_METHOD("add_neighbor_rule", "tile1_id", "orientation1", "tile2_id", "orientation2"),
                        &GDTilingWFCv2::add_neighbor_rule);

    // Run
    ClassDB::bind_method(D_METHOD("run"), &GDTilingWFCv2::run);
    ClassDB::bind_method(D_METHOD("clear"), &GDTilingWFCv2::clear);
    ClassDB::bind_method(D_METHOD("get_configuration"), &GDTilingWFCv2::get_configuration);
}

void GDTilingWFCv2::set_size(int p_width, int p_height) {
    width = p_width;
    height = p_height;
}

void GDTilingWFCv2::set_seed(int p_seed) {
    seed = p_seed;
}

void GDTilingWFCv2::set_periodic(bool p_periodic) {
    periodic = p_periodic;
}

void GDTilingWFCv2::set_configuration(Ref<WFCConfiguration> p_config) {
    config = p_config;
}

void GDTilingWFCv2::enable_debug(bool enabled) {
    debug_mode = enabled;
}

void GDTilingWFCv2::use_connection_system() {
    if (config.is_valid()) {
        config->enable_connection_system(true);
    }
}

void GDTilingWFCv2::add_connected_tile(int tile_id, Dictionary connections, float weight) {
    if (config.is_valid()) {
        config->add_connected_tile(tile_id, connections, weight);
    }
}

void GDTilingWFCv2::set_tile_stamp(int tile_id, PackedInt32Array stamp_pattern,
                                   int stamp_width, int stamp_height) {
    if (config.is_valid()) {
        config->set_tile_stamp(tile_id, stamp_pattern, stamp_width, stamp_height);
    }
}

void GDTilingWFCv2::auto_generate_rules() {
    if (config.is_valid()) {
        config->auto_generate_neighbor_rules();
    }
}

void GDTilingWFCv2::add_tile(int tile_id, PackedInt32Array tile_data, int tile_size,
                             SymmetryType symmetry, float weight) {
    if (config.is_valid()) {
        config->add_tile(tile_id, tile_data, tile_size, symmetry, weight);
    }
}

void GDTilingWFCv2::add_neighbor_rule(int tile1_id, int orientation1,
                                      int tile2_id, int orientation2) {
    if (config.is_valid()) {
        config->add_neighbor_rule(tile1_id, orientation1, tile2_id, orientation2);
    }
}

Ref<WFCResult> GDTilingWFCv2::run() {
    Ref<WFCResult> result;
    result.instantiate();

    if (!config.is_valid()) {
        result->_set_failure("No configuration set", Vector2i(0, 0));
        return result;
    }

    const std::vector<WFCConfiguration::TileDefinition>& tiles = config->get_tiles();
    const std::vector<WFCConfiguration::NeighborRule>& rules = config->get_rules();

    if (tiles.empty()) {
        result->_set_failure("No tiles defined", Vector2i(0, 0));
        return result;
    }

    if (rules.empty()) {
        result->_set_failure("No neighbor rules defined", Vector2i(0, 0));
        return result;
    }

    try {
        // ====================================================================
        // STEP 1: Convert tiles to WFC format
        // ====================================================================
        std::vector<Tile<int>> wfc_tiles;
        std::map<int, int> tile_id_to_index;  // Map tile_id -> index in wfc_tiles

        for (size_t i = 0; i < tiles.size(); i++) {
            const auto& tile_def = tiles[i];

            // Create Array2D from tile_data
            int size = tile_def.tile_size;
            Array2D<int> tile_array(size, size);

            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    int index = y * size + x;
                    if (index < tile_def.tile_data.size()) {
                        tile_array.get(y, x) = tile_def.tile_data[index];
                    }
                }
            }

            // Convert symmetry
            Symmetry sym;
            switch (tile_def.symmetry) {
                case WFCConfiguration::SYMMETRY_X: sym = Symmetry::X; break;
                case WFCConfiguration::SYMMETRY_I: sym = Symmetry::I; break;
                case WFCConfiguration::SYMMETRY_BACKSLASH: sym = Symmetry::backslash; break;
                case WFCConfiguration::SYMMETRY_T: sym = Symmetry::T; break;
                case WFCConfiguration::SYMMETRY_L: sym = Symmetry::L; break;
                case WFCConfiguration::SYMMETRY_P: sym = Symmetry::P; break;
                default: sym = Symmetry::X; break;
            }

            wfc_tiles.push_back(Tile<int>(tile_array, sym, tile_def.weight));
            tile_id_to_index[tile_def.tile_id] = i;
        }

        // ====================================================================
        // STEP 2: Convert neighbor rules
        // ====================================================================
        std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>> wfc_neighbors;

        for (const auto& rule : rules) {
            // Map tile IDs to indices
            auto it1 = tile_id_to_index.find(rule.tile1_id);
            auto it2 = tile_id_to_index.find(rule.tile2_id);

            if (it1 != tile_id_to_index.end() && it2 != tile_id_to_index.end()) {
                wfc_neighbors.push_back(std::make_tuple(
                    it1->second,  // tile1 index
                    rule.orientation1,
                    it2->second,  // tile2 index
                    rule.orientation2
                ));
            }
        }

        if (debug_mode) {
            UtilityFunctions::print("WFCv2: Running with ", (int)wfc_tiles.size(), " tiles and ", (int)wfc_neighbors.size(), " rules");
        }

        // ====================================================================
        // STEP 3: Run WFC algorithm
        // ====================================================================
        TilingWFCOptions options;
        options.periodic_output = periodic;

        TilingWFC<int> wfc(wfc_tiles, wfc_neighbors, height, width, options, seed);
        std::optional<Array2D<int>> output = wfc.run();

        if (!output.has_value()) {
            result->_set_failure("WFC contradiction - no valid solution", Vector2i(-1, -1));
            return result;
        }

        // ====================================================================
        // STEP 4: Convert WFC output to PackedInt32Array
        // ====================================================================
        Array2D<int>& output_array = output.value();
        int output_height = output_array.height;
        int output_width = output_array.width;

        PackedInt32Array wfc_result;
        wfc_result.resize(output_height * output_width);

        for (int y = 0; y < output_height; y++) {
            for (int x = 0; x < output_width; x++) {
                int tile_index = output_array.get(y, x);
                // Convert back to tile_id
                int tile_id = tiles[tile_index].tile_id;
                wfc_result[y * output_width + x] = tile_id;
            }
        }

        result->_set_wfc_data(wfc_result, output_width, output_height);

        // ====================================================================
        // STEP 5: Expand stamps if configured
        // ====================================================================
        int stamp_size = config->get_stamp_size();

        if (stamp_size > 0) {
            // Check if all tiles have stamps
            bool all_have_stamps = true;
            for (const auto& tile : tiles) {
                if (!tile.has_stamp) {
                    all_have_stamps = false;
                    break;
                }
            }

            if (all_have_stamps) {
                int expanded_width = output_width * stamp_size;
                int expanded_height = output_height * stamp_size;

                PackedInt32Array expanded_tiles;
                expanded_tiles.resize(expanded_width * expanded_height);
                // Initialize to -1 to distinguish unset tiles from actual floor tiles (0)
                expanded_tiles.fill(-1);

                // Expand each WFC tile to its stamp
                for (int wfc_y = 0; wfc_y < output_height; wfc_y++) {
                    for (int wfc_x = 0; wfc_x < output_width; wfc_x++) {
                        int tile_id = wfc_result[wfc_y * output_width + wfc_x];

                        // Find tile definition
                        const WFCConfiguration::TileDefinition* tile_def = nullptr;
                        for (const auto& t : tiles) {
                            if (t.tile_id == tile_id) {
                                tile_def = &t;
                                break;
                            }
                        }

                        if (!tile_def || !tile_def->has_stamp) {
                            continue;
                        }

                        // Place stamp
                        int base_x = wfc_x * stamp_size;
                        int base_y = wfc_y * stamp_size;

                        for (int local_y = 0; local_y < tile_def->stamp_height; local_y++) {
                            for (int local_x = 0; local_x < tile_def->stamp_width; local_x++) {
                                int stamp_index = local_y * tile_def->stamp_width + local_x;
                                if (stamp_index < tile_def->stamp_pattern.size()) {
                                    int world_x = base_x + local_x;
                                    int world_y = base_y + local_y;

                                    if (world_x < expanded_width && world_y < expanded_height) {
                                        int tile_value = tile_def->stamp_pattern[stamp_index];
                                        expanded_tiles[world_y * expanded_width + world_x] = tile_value;
                                    }
                                }
                            }
                        }
                    }
                }

                result->_set_expanded_data(expanded_tiles, expanded_width, expanded_height, stamp_size);

                if (debug_mode) {
                    UtilityFunctions::print("WFCv2: Expanded from ", output_width, "x", output_height,
                                          " to ", expanded_width, "x", expanded_height,
                                          " (", stamp_size, "x", stamp_size, " stamps)");
                }
            } else {
                UtilityFunctions::push_warning("WFCv2: Stamp size set but not all tiles have stamps defined");
            }
        }

        if (debug_mode) {
            UtilityFunctions::print("WFCv2: Success! Generated ", output_width, "x", output_height, " dungeon");
        }

    } catch (const std::exception& e) {
        result->_set_failure(String("WFC error: ") + e.what(), Vector2i(0, 0));
    }

    return result;
}

void GDTilingWFCv2::clear() {
    if (config.is_valid()) {
        config->clear();
    }
}
