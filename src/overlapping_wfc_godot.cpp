#include "overlapping_wfc_godot.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <chrono>

#include "../fast-wfc/src/include/overlapping_wfc.hpp"
#include "../fast-wfc/src/include/utils/array2D.hpp"

using namespace godot;

// ============================================================================
// OverlappingWFCResult Implementation
// ============================================================================

OverlappingWFCResult::OverlappingWFCResult() :
    output_width(0), output_height(0),
    expanded_width(0), expanded_height(0),
    has_stamps(false), stamp_size(0),
    success(false) {
}

OverlappingWFCResult::~OverlappingWFCResult() {
}

void OverlappingWFCResult::_bind_methods() {
    // Result queries
    ClassDB::bind_method(D_METHOD("is_success"), &OverlappingWFCResult::is_success);
    ClassDB::bind_method(D_METHOD("get_failure_reason"), &OverlappingWFCResult::get_failure_reason);

    // Dimensions
    ClassDB::bind_method(D_METHOD("get_output_width"), &OverlappingWFCResult::get_output_width);
    ClassDB::bind_method(D_METHOD("get_output_height"), &OverlappingWFCResult::get_output_height);
    ClassDB::bind_method(D_METHOD("get_expanded_width"), &OverlappingWFCResult::get_expanded_width);
    ClassDB::bind_method(D_METHOD("get_expanded_height"), &OverlappingWFCResult::get_expanded_height);

    // Tile access
    ClassDB::bind_method(D_METHOD("get_pattern_at", "x", "y"), &OverlappingWFCResult::get_pattern_at);
    ClassDB::bind_method(D_METHOD("get_tile_at", "x", "y"), &OverlappingWFCResult::get_tile_at);
    ClassDB::bind_method(D_METHOD("get_expanded_tile_at", "x", "y"), &OverlappingWFCResult::get_expanded_tile_at);
    ClassDB::bind_method(D_METHOD("get_pattern_output"), &OverlappingWFCResult::get_pattern_output);
    ClassDB::bind_method(D_METHOD("get_tile_output"), &OverlappingWFCResult::get_tile_output);
    ClassDB::bind_method(D_METHOD("get_expanded_output"), &OverlappingWFCResult::get_expanded_output);

    // Helper methods
    ClassDB::bind_method(D_METHOD("get_floor_positions", "floor_value"), &OverlappingWFCResult::get_floor_positions, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("get_wall_positions", "wall_value"), &OverlappingWFCResult::get_wall_positions, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("get_statistics"), &OverlappingWFCResult::get_statistics);

    // Internal
    ClassDB::bind_method(D_METHOD("_set_pattern_data", "patterns", "width", "height"), &OverlappingWFCResult::_set_pattern_data);
    ClassDB::bind_method(D_METHOD("_set_tile_data", "tiles", "width", "height"), &OverlappingWFCResult::_set_tile_data);
    ClassDB::bind_method(D_METHOD("_set_expanded_data", "tiles", "width", "height", "stamp_size"), &OverlappingWFCResult::_set_expanded_data);
    ClassDB::bind_method(D_METHOD("_set_failure", "reason"), &OverlappingWFCResult::_set_failure);
}

int OverlappingWFCResult::get_pattern_at(int x, int y) const {
    if (x < 0 || x >= output_width || y < 0 || y >= output_height) {
        return -1;
    }
    return pattern_output[y * output_width + x];
}

int OverlappingWFCResult::get_tile_at(int x, int y) const {
    if (tile_output.is_empty()) {
        return get_pattern_at(x, y);
    }
    if (x < 0 || x >= output_width || y < 0 || y >= output_height) {
        return -1;
    }
    return tile_output[y * output_width + x];
}

int OverlappingWFCResult::get_expanded_tile_at(int x, int y) const {
    if (!has_stamps) {
        return get_tile_at(x, y);
    }
    if (x < 0 || x >= expanded_width || y < 0 || y >= expanded_height) {
        return -1;
    }
    return expanded_output[y * expanded_width + x];
}

PackedVector2Array OverlappingWFCResult::get_floor_positions(int floor_tile_value) const {
    PackedVector2Array positions;

    // Use expanded output if available, otherwise tile output
    const PackedInt32Array& tiles = has_stamps ? expanded_output :
                                    (!tile_output.is_empty() ? tile_output : pattern_output);
    int w = has_stamps ? expanded_width : output_width;
    int h = has_stamps ? expanded_height : output_height;

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

PackedVector2Array OverlappingWFCResult::get_wall_positions(int wall_tile_value) const {
    PackedVector2Array positions;

    const PackedInt32Array& tiles = has_stamps ? expanded_output :
                                    (!tile_output.is_empty() ? tile_output : pattern_output);
    int w = has_stamps ? expanded_width : output_width;
    int h = has_stamps ? expanded_height : output_height;

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

Dictionary OverlappingWFCResult::get_statistics() const {
    Dictionary stats;
    stats["output_width"] = output_width;
    stats["output_height"] = output_height;
    stats["pattern_count"] = pattern_output.size();

    if (has_stamps) {
        stats["expanded_width"] = expanded_width;
        stats["expanded_height"] = expanded_height;
        stats["stamp_size"] = stamp_size;
    }

    return stats;
}

void OverlappingWFCResult::_set_pattern_data(PackedInt32Array patterns, int width, int height) {
    pattern_output = patterns;
    output_width = width;
    output_height = height;
    success = true;
}

void OverlappingWFCResult::_set_tile_data(PackedInt32Array tiles, int width, int height) {
    tile_output = tiles;
    output_width = width;
    output_height = height;
}

void OverlappingWFCResult::_set_expanded_data(PackedInt32Array tiles, int width, int height, int p_stamp_size) {
    expanded_output = tiles;
    expanded_width = width;
    expanded_height = height;
    has_stamps = true;
    stamp_size = p_stamp_size;
}

void OverlappingWFCResult::_set_failure(String reason) {
    success = false;
    failure_reason = reason;
}

// ============================================================================
// OverlappingWFCGenerator Implementation
// ============================================================================

OverlappingWFCGenerator::OverlappingWFCGenerator() :
    output_width(48), output_height(48),
    pattern_size(3), symmetry(8),
    seed(0), use_seed(false),
    periodic_input(false), periodic_output(false),
    ground_mode(false),
    use_stamps(false), stamp_size(3),
    debug_mode(false) {
}

OverlappingWFCGenerator::~OverlappingWFCGenerator() {
}

void OverlappingWFCGenerator::_bind_methods() {
    // Basic settings
    ClassDB::bind_method(D_METHOD("set_seed_image", "image"), &OverlappingWFCGenerator::set_seed_image);
    ClassDB::bind_method(D_METHOD("get_seed_image"), &OverlappingWFCGenerator::get_seed_image);

    ClassDB::bind_method(D_METHOD("set_output_size", "width", "height"), &OverlappingWFCGenerator::set_output_size);
    ClassDB::bind_method(D_METHOD("set_output_width", "width"), &OverlappingWFCGenerator::set_output_width);
    ClassDB::bind_method(D_METHOD("set_output_height", "height"), &OverlappingWFCGenerator::set_output_height);
    ClassDB::bind_method(D_METHOD("get_output_width"), &OverlappingWFCGenerator::get_output_width);
    ClassDB::bind_method(D_METHOD("get_output_height"), &OverlappingWFCGenerator::get_output_height);

    ClassDB::bind_method(D_METHOD("set_pattern_size", "size"), &OverlappingWFCGenerator::set_pattern_size);
    ClassDB::bind_method(D_METHOD("get_pattern_size"), &OverlappingWFCGenerator::get_pattern_size);

    ClassDB::bind_method(D_METHOD("set_symmetry", "symmetry"), &OverlappingWFCGenerator::set_symmetry);
    ClassDB::bind_method(D_METHOD("get_symmetry"), &OverlappingWFCGenerator::get_symmetry);

    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &OverlappingWFCGenerator::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &OverlappingWFCGenerator::get_seed);

    ClassDB::bind_method(D_METHOD("set_use_seed", "use"), &OverlappingWFCGenerator::set_use_seed);
    ClassDB::bind_method(D_METHOD("get_use_seed"), &OverlappingWFCGenerator::get_use_seed);

    ClassDB::bind_method(D_METHOD("set_periodic_input", "periodic"), &OverlappingWFCGenerator::set_periodic_input);
    ClassDB::bind_method(D_METHOD("get_periodic_input"), &OverlappingWFCGenerator::get_periodic_input);

    ClassDB::bind_method(D_METHOD("set_periodic_output", "periodic"), &OverlappingWFCGenerator::set_periodic_output);
    ClassDB::bind_method(D_METHOD("get_periodic_output"), &OverlappingWFCGenerator::get_periodic_output);

    ClassDB::bind_method(D_METHOD("set_ground_mode", "enabled"), &OverlappingWFCGenerator::set_ground_mode);
    ClassDB::bind_method(D_METHOD("get_ground_mode"), &OverlappingWFCGenerator::get_ground_mode);

    // Stamp system
    ClassDB::bind_method(D_METHOD("enable_stamps", "enabled"), &OverlappingWFCGenerator::enable_stamps);
    ClassDB::bind_method(D_METHOD("get_stamps_enabled"), &OverlappingWFCGenerator::get_stamps_enabled);

    ClassDB::bind_method(D_METHOD("set_stamp_size", "size"), &OverlappingWFCGenerator::set_stamp_size);
    ClassDB::bind_method(D_METHOD("get_stamp_size"), &OverlappingWFCGenerator::get_stamp_size);

    ClassDB::bind_method(D_METHOD("add_pattern_to_tile_mapping", "color_value", "tile_id"),
                        &OverlappingWFCGenerator::add_pattern_to_tile_mapping);
    ClassDB::bind_method(D_METHOD("set_tile_stamp", "tile_id", "stamp_pattern", "stamp_width", "stamp_height"),
                        &OverlappingWFCGenerator::set_tile_stamp);

    ClassDB::bind_method(D_METHOD("clear_pattern_mappings"), &OverlappingWFCGenerator::clear_pattern_mappings);
    ClassDB::bind_method(D_METHOD("clear_tile_stamps"), &OverlappingWFCGenerator::clear_tile_stamps);

    // Quick setup
    ClassDB::bind_method(D_METHOD("setup_default_dungeon_mapping"), &OverlappingWFCGenerator::setup_default_dungeon_mapping);
    ClassDB::bind_method(D_METHOD("setup_default_dungeon_stamps"), &OverlappingWFCGenerator::setup_default_dungeon_stamps);

    // Generation
    ClassDB::bind_method(D_METHOD("generate"), &OverlappingWFCGenerator::generate);

    // Utility
    ClassDB::bind_method(D_METHOD("enable_debug", "enabled"), &OverlappingWFCGenerator::enable_debug);
    ClassDB::bind_method(D_METHOD("get_debug_enabled"), &OverlappingWFCGenerator::get_debug_enabled);
    ClassDB::bind_method(D_METHOD("get_configuration_info"), &OverlappingWFCGenerator::get_configuration_info);

    // Properties
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "seed_image", PROPERTY_HINT_RESOURCE_TYPE, "Image"),
                "set_seed_image", "get_seed_image");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "output_width", PROPERTY_HINT_RANGE, "8,512,1"),
                "set_output_width", "get_output_width");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "output_height", PROPERTY_HINT_RANGE, "8,512,1"),
                "set_output_height", "get_output_height");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "pattern_size", PROPERTY_HINT_RANGE, "2,5,1"),
                "set_pattern_size", "get_pattern_size");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "symmetry", PROPERTY_HINT_RANGE, "1,8,1"),
                "set_symmetry", "get_symmetry");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_seed"), "set_use_seed", "get_use_seed");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "periodic_input"), "set_periodic_input", "get_periodic_input");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "periodic_output"), "set_periodic_output", "get_periodic_output");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ground_mode"), "set_ground_mode", "get_ground_mode");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_stamps"), "enable_stamps", "get_stamps_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stamp_size", PROPERTY_HINT_RANGE, "1,5,1"),
                "set_stamp_size", "get_stamp_size");
}

void OverlappingWFCGenerator::set_seed_image(const Ref<Image>& p_image) {
    seed_image = p_image;
}

void OverlappingWFCGenerator::set_output_size(int width, int height) {
    output_width = width;
    output_height = height;
}

void OverlappingWFCGenerator::set_output_width(int width) {
    output_width = width;
}

void OverlappingWFCGenerator::set_output_height(int height) {
    output_height = height;
}

void OverlappingWFCGenerator::set_pattern_size(int size) {
    pattern_size = std::max(2, std::min(5, size));
}

void OverlappingWFCGenerator::set_symmetry(int p_symmetry) {
    symmetry = std::max(1, std::min(8, p_symmetry));
}

void OverlappingWFCGenerator::set_seed(int p_seed) {
    seed = p_seed;
}

void OverlappingWFCGenerator::set_use_seed(bool p_use) {
    use_seed = p_use;
}

void OverlappingWFCGenerator::set_periodic_input(bool periodic) {
    periodic_input = periodic;
}

void OverlappingWFCGenerator::set_periodic_output(bool periodic) {
    periodic_output = periodic;
}

void OverlappingWFCGenerator::set_ground_mode(bool enabled) {
    ground_mode = enabled;
}

void OverlappingWFCGenerator::enable_stamps(bool enabled) {
    use_stamps = enabled;
}

void OverlappingWFCGenerator::set_stamp_size(int size) {
    stamp_size = std::max(1, std::min(5, size));
}

void OverlappingWFCGenerator::add_pattern_to_tile_mapping(int color_value, int tile_id) {
    pattern_to_tile_map[color_value] = tile_id;
}

void OverlappingWFCGenerator::set_tile_stamp(int tile_id, const PackedInt32Array& stamp_pattern,
                                             int stamp_width, int stamp_height) {
    Dictionary stamp_data;
    stamp_data["pattern"] = stamp_pattern;
    stamp_data["width"] = stamp_width;
    stamp_data["height"] = stamp_height;
    tile_stamps[tile_id] = stamp_data;
}

void OverlappingWFCGenerator::clear_pattern_mappings() {
    pattern_to_tile_map.clear();
}

void OverlappingWFCGenerator::clear_tile_stamps() {
    tile_stamps.clear();
}

void OverlappingWFCGenerator::setup_default_dungeon_mapping() {
    // Map black (0x000000) to floor (0)
    // Map white (0xFFFFFF) to wall (1)
    add_pattern_to_tile_mapping(0x000000, 0);  // Black -> Floor
    add_pattern_to_tile_mapping(0xFFFFFF, 1);  // White -> Wall

    if (debug_mode) {
        UtilityFunctions::print("OverlappingWFC: Set up default dungeon mapping (black=floor, white=wall)");
    }
}

void OverlappingWFCGenerator::setup_default_dungeon_stamps() {
    // Floor stamp (all floor tiles)
    PackedInt32Array floor_stamp;
    floor_stamp.resize(stamp_size * stamp_size);
    for (int i = 0; i < floor_stamp.size(); i++) {
        floor_stamp[i] = 0;  // All floor
    }
    set_tile_stamp(0, floor_stamp, stamp_size, stamp_size);

    // Wall stamp (all wall tiles)
    PackedInt32Array wall_stamp;
    wall_stamp.resize(stamp_size * stamp_size);
    for (int i = 0; i < wall_stamp.size(); i++) {
        wall_stamp[i] = 1;  // All wall
    }
    set_tile_stamp(1, wall_stamp, stamp_size, stamp_size);

    if (debug_mode) {
        UtilityFunctions::print("OverlappingWFC: Set up default ", stamp_size, "x", stamp_size, " dungeon stamps");
    }
}

Dictionary OverlappingWFCGenerator::get_configuration_info() const {
    Dictionary info;
    info["has_seed_image"] = seed_image.is_valid();
    if (seed_image.is_valid()) {
        info["image_width"] = seed_image->get_width();
        info["image_height"] = seed_image->get_height();
    }
    info["output_width"] = output_width;
    info["output_height"] = output_height;
    info["pattern_size"] = pattern_size;
    info["symmetry"] = symmetry;
    info["use_stamps"] = use_stamps;
    info["stamp_size"] = stamp_size;
    info["pattern_mappings_count"] = pattern_to_tile_map.size();
    info["tile_stamps_count"] = tile_stamps.size();
    return info;
}

void OverlappingWFCGenerator::enable_debug(bool enabled) {
    debug_mode = enabled;
}

Ref<OverlappingWFCResult> OverlappingWFCGenerator::generate() {
    Ref<OverlappingWFCResult> result;
    result.instantiate();

    // Validation
    if (!seed_image.is_valid()) {
        result->_set_failure("No seed image provided");
        return result;
    }

    if (seed_image->is_empty()) {
        result->_set_failure("Seed image is empty");
        return result;
    }

    try {
        // ====================================================================
        // STEP 1: Convert Godot Image to Array2D<int>
        // ====================================================================
        int img_width = seed_image->get_width();
        int img_height = seed_image->get_height();

        if (debug_mode) {
            UtilityFunctions::print("OverlappingWFC: Processing ", img_width, "x", img_height, " seed image");
        }

        Array2D<int> input_array(img_height, img_width);

        // Convert each pixel to an integer color value
        for (int y = 0; y < img_height; y++) {
            for (int x = 0; x < img_width; x++) {
                Color pixel = seed_image->get_pixel(x, y);
                // Convert to RGB int (ignore alpha for now)
                int color_value = (int(pixel.r * 255) << 16) |
                                 (int(pixel.g * 255) << 8) |
                                 int(pixel.b * 255);
                input_array.get(y, x) = color_value;
            }
        }

        // ====================================================================
        // STEP 2: Set up WFC options
        // ====================================================================
        OverlappingWFCOptions options;
        options.periodic_input = periodic_input;
        options.periodic_output = periodic_output;
        options.out_height = output_height;
        options.out_width = output_width;
        options.symmetry = symmetry;
        options.ground = ground_mode;
        options.pattern_size = pattern_size;

        // ====================================================================
        // STEP 3: Run Overlapping WFC
        // ====================================================================
        int wfc_seed = use_seed ? seed : (int)std::chrono::system_clock::now().time_since_epoch().count();
        OverlappingWFC<int> wfc(input_array, options, wfc_seed);

        if (debug_mode) {
            UtilityFunctions::print("OverlappingWFC: Running with pattern_size=", pattern_size,
                                  ", symmetry=", symmetry, ", seed=", wfc_seed);
        }

        std::optional<Array2D<int>> wfc_output = wfc.run();

        if (!wfc_output.has_value()) {
            result->_set_failure("WFC contradiction - no valid solution found");
            return result;
        }

        // ====================================================================
        // STEP 4: Convert WFC output to PackedInt32Array
        // ====================================================================
        Array2D<int>& output_array = wfc_output.value();

        PackedInt32Array pattern_result;
        pattern_result.resize(output_height * output_width);

        for (int y = 0; y < output_height; y++) {
            for (int x = 0; x < output_width; x++) {
                pattern_result[y * output_width + x] = output_array.get(y, x);
            }
        }

        result->_set_pattern_data(pattern_result, output_width, output_height);

        // ====================================================================
        // STEP 5: Apply pattern-to-tile mapping (if configured)
        // ====================================================================
        if (!pattern_to_tile_map.is_empty()) {
            PackedInt32Array tile_result;
            tile_result.resize(output_height * output_width);

            for (int y = 0; y < output_height; y++) {
                for (int x = 0; x < output_width; x++) {
                    int pattern_value = pattern_result[y * output_width + x];

                    // Map pattern to tile ID
                    if (pattern_to_tile_map.has(pattern_value)) {
                        tile_result[y * output_width + x] = pattern_to_tile_map[pattern_value];
                    } else {
                        // Default to pattern value if no mapping exists
                        tile_result[y * output_width + x] = pattern_value;
                    }
                }
            }

            result->_set_tile_data(tile_result, output_width, output_height);

            if (debug_mode) {
                UtilityFunctions::print("OverlappingWFC: Applied pattern-to-tile mappings");
            }
        }

        // ====================================================================
        // STEP 6: Expand with stamps (if configured)
        // ====================================================================
        if (use_stamps && !tile_stamps.is_empty()) {
            int expanded_width = output_width * stamp_size;
            int expanded_height = output_height * stamp_size;

            PackedInt32Array expanded_result;
            expanded_result.resize(expanded_width * expanded_height);
            // Initialize to -1 to distinguish unset tiles from actual floor tiles (0)
            expanded_result.fill(-1);

            // Get the tile array (use mapped tiles if available, otherwise patterns)
            const PackedInt32Array& tile_array = pattern_to_tile_map.is_empty() ?
                                                 pattern_result : result->get_tile_output();

            // Expand each tile to its stamp
            for (int wfc_y = 0; wfc_y < output_height; wfc_y++) {
                for (int wfc_x = 0; wfc_x < output_width; wfc_x++) {
                    int tile_id = tile_array[wfc_y * output_width + wfc_x];

                    // Get stamp for this tile
                    if (tile_stamps.has(tile_id)) {
                        Dictionary stamp_data = tile_stamps[tile_id];
                        PackedInt32Array stamp_pattern = stamp_data["pattern"];
                        int stamp_w = stamp_data["width"];
                        int stamp_h = stamp_data["height"];

                        // Place stamp
                        int base_x = wfc_x * stamp_size;
                        int base_y = wfc_y * stamp_size;

                        for (int local_y = 0; local_y < stamp_h; local_y++) {
                            for (int local_x = 0; local_x < stamp_w; local_x++) {
                                int stamp_index = local_y * stamp_w + local_x;
                                if (stamp_index < stamp_pattern.size()) {
                                    int world_x = base_x + local_x;
                                    int world_y = base_y + local_y;

                                    if (world_x < expanded_width && world_y < expanded_height) {
                                        int tile_value = stamp_pattern[stamp_index];
                                        expanded_result[world_y * expanded_width + world_x] = tile_value;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            result->_set_expanded_data(expanded_result, expanded_width, expanded_height, stamp_size);

            if (debug_mode) {
                UtilityFunctions::print("OverlappingWFC: Expanded from ", output_width, "x", output_height,
                                      " to ", expanded_width, "x", expanded_height, " with stamps");
            }
        }

        if (debug_mode) {
            UtilityFunctions::print("OverlappingWFC: Generation successful!");
        }

    } catch (const std::exception& e) {
        result->_set_failure(String("WFC error: ") + e.what());
    }

    return result;
}
