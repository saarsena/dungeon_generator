#include "gdwfc.h"
#include <tuple>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "../tiling-wfc/include/tiling_wfc.hpp"
#include "../tiling-wfc/include/utils/array2D.hpp"

void GDTilingWFC::_bind_methods() {
    // Configuration methods
    ClassDB::bind_method(D_METHOD("set_size", "width", "height"), &GDTilingWFC::set_size);
    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &GDTilingWFC::set_seed);
    ClassDB::bind_method(D_METHOD("set_periodic", "periodic"), &GDTilingWFC::set_periodic);

    // Tile and rule methods
    ClassDB::bind_method(D_METHOD("add_tile", "tile_id", "tile_data", "tile_size", "symmetry", "weight"),
                         &GDTilingWFC::add_tile);
    ClassDB::bind_method(D_METHOD("add_neighbor_rule", "tile1_id", "orientation1", "tile2_id", "orientation2"),
                         &GDTilingWFC::add_neighbor_rule);

    // Execution and results
    ClassDB::bind_method(D_METHOD("run"), &GDTilingWFC::run);
    ClassDB::bind_method(D_METHOD("clear"), &GDTilingWFC::clear);
    ClassDB::bind_method(D_METHOD("get_output_width"), &GDTilingWFC::get_output_width);
    ClassDB::bind_method(D_METHOD("get_output_height"), &GDTilingWFC::get_output_height);

    // Bind symmetry constants
    BIND_ENUM_CONSTANT(SYMMETRY_X);
    BIND_ENUM_CONSTANT(SYMMETRY_I);
    BIND_ENUM_CONSTANT(SYMMETRY_BACKSLASH);
    BIND_ENUM_CONSTANT(SYMMETRY_T);
    BIND_ENUM_CONSTANT(SYMMETRY_L);
    BIND_ENUM_CONSTANT(SYMMETRY_P);
}

GDTilingWFC::GDTilingWFC()
    : width(10), height(10), seed(0), periodic(false) {
}

GDTilingWFC::~GDTilingWFC() {
}

void GDTilingWFC::set_size(int p_width, int p_height) {
    width = p_width;
    height = p_height;
}

void GDTilingWFC::set_seed(int p_seed) {
    seed = p_seed;
}

void GDTilingWFC::set_periodic(bool p_periodic) {
    periodic = p_periodic;
}

void GDTilingWFC::add_tile(int tile_id, PackedInt32Array tile_data, int tile_size, SymmetryType symmetry, float weight) {
    TileDefinition tile_def;
    tile_def.tile_id = tile_id;
    tile_def.tile_data = tile_data;
    tile_def.tile_size = tile_size;
    tile_def.symmetry = symmetry;
    tile_def.weight = weight;

    tiles.push_back(tile_def);
}

void GDTilingWFC::add_neighbor_rule(int tile1_id, int orientation1, int tile2_id, int orientation2) {
    NeighborRule rule;
    rule.tile1_id = tile1_id;
    rule.orientation1 = orientation1;
    rule.tile2_id = tile2_id;
    rule.orientation2 = orientation2;

    neighbor_rules.push_back(rule);
}

void GDTilingWFC::clear() {
    tiles.clear();
    neighbor_rules.clear();
}

PackedInt32Array GDTilingWFC::run() {
    PackedInt32Array result;

    if (tiles.empty()) {
        UtilityFunctions::print("Error: No tiles defined");
        return result;
    }

    if (neighbor_rules.empty()) {
        UtilityFunctions::print("Error: No neighbor rules defined");
        return result;
    }

    try {
        // Convert GD tiles to WFC Tile<int> objects
        std::vector<Tile<int>> wfc_tiles;

        for (const auto& tile_def : tiles) {
            // Create Array2D from the tile data
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
                case SYMMETRY_X: sym = Symmetry::X; break;
                case SYMMETRY_I: sym = Symmetry::I; break;
                case SYMMETRY_BACKSLASH: sym = Symmetry::backslash; break;
                case SYMMETRY_T: sym = Symmetry::T; break;
                case SYMMETRY_L: sym = Symmetry::L; break;
                case SYMMETRY_P: sym = Symmetry::P; break;
                default: sym = Symmetry::X; break;
            }

            wfc_tiles.push_back(Tile<int>(tile_array, sym, tile_def.weight));
        }

        // Convert neighbor rules
        std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>> wfc_neighbors;
        for (const auto& rule : neighbor_rules) {
            wfc_neighbors.push_back(std::make_tuple(
                rule.tile1_id,
                rule.orientation1,
                rule.tile2_id,
                rule.orientation2
            ));
        }

        // Create options
        TilingWFCOptions options;
        options.periodic_output = periodic;

        // Run WFC
        TilingWFC<int> wfc(wfc_tiles, wfc_neighbors, height, width, options, seed);
        std::optional<Array2D<int>> output = wfc.run();

        if (output.has_value()) {
            // Convert result to PackedInt32Array
            Array2D<int>& output_array = output.value();
            int output_height = output_array.height;
            int output_width = output_array.width;

            result.resize(output_height * output_width);

            for (int y = 0; y < output_height; y++) {
                for (int x = 0; x < output_width; x++) {
                    result[y * output_width + x] = output_array.get(y, x);
                }
            }

            UtilityFunctions::print("WFC succeeded! Output size: ", output_width, "x", output_height);
            UtilityFunctions::print("  Total neighbor rules provided: ", wfc_neighbors.size());
        } else {
            UtilityFunctions::print("WFC failed - contradiction encountered");
        }

    } catch (const std::exception& e) {
        UtilityFunctions::print("WFC error: ", e.what());
    }

    return result;
}

int GDTilingWFC::get_output_width() const {
    if (tiles.empty()) return 0;
    return width * tiles[0].tile_size;
}

int GDTilingWFC::get_output_height() const {
    if (tiles.empty()) return 0;
    return height * tiles[0].tile_size;
}
