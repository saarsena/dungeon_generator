#include "tiling_wfc.hpp"
#include <iostream>
#include <vector>

/**
 * Simple example demonstrating how to use the Tiling WFC library.
 * This creates a simple 2-tile system where tiles can be placed adjacent to each other.
 */

int main() {
    std::cout << "Tiling WFC Simple Example\n";
    std::cout << "==========================\n\n";

    // Define tile size (e.g., 1x1 for simplicity)
    constexpr unsigned tile_size = 1;

    // Create two simple tiles (represented as integers 0 and 1)
    // Tile 0: value 0
    Array2D<int> tile_data_0(tile_size, tile_size, 0);
    // Tile 1: value 1
    Array2D<int> tile_data_1(tile_size, tile_size, 1);

    // Create tiles with different symmetries and weights
    std::vector<Tile<int>> tiles = {
        Tile<int>(tile_data_0, Symmetry::X, 1.0),  // Tile 0: no rotation, weight 1.0
        Tile<int>(tile_data_1, Symmetry::X, 1.0)   // Tile 1: no rotation, weight 1.0
    };

    // Define neighbor rules: which tiles can be placed next to each other
    // Format: (tile1_id, tile1_orientation, tile2_id, tile2_orientation)
    // This means tile1 can have tile2 to its right
    std::vector<std::tuple<unsigned, unsigned, unsigned, unsigned>> neighbors = {
        // Tile 0 can have tile 0 to its right
        std::make_tuple(0, 0, 0, 0),
        // Tile 0 can have tile 1 to its right
        std::make_tuple(0, 0, 1, 0),
        // Tile 1 can have tile 0 to its right
        std::make_tuple(1, 0, 0, 0),
        // Tile 1 can have tile 1 to its right
        std::make_tuple(1, 0, 1, 0)
    };

    // Set up WFC options
    TilingWFCOptions options;
    options.periodic_output = false;  // Non-periodic (has edges)

    // Output dimensions (in tiles)
    const unsigned output_height = 5;
    const unsigned output_width = 5;

    // Seed for random generation
    const int seed = 12345;

    std::cout << "Creating a " << output_height << "x" << output_width
              << " tiled output...\n\n";

    // Create the Tiling WFC instance
    TilingWFC<int> wfc(tiles, neighbors, output_height, output_width, options, seed);

    // Run the algorithm
    std::cout << "Running WFC algorithm...\n";
    auto result = wfc.run();

    if (result.has_value()) {
        std::cout << "Success! Generated output:\n\n";

        // Print the result
        Array2D<int> output = result.value();
        for (unsigned i = 0; i < output.height; i++) {
            for (unsigned j = 0; j < output.width; j++) {
                std::cout << output.get(i, j) << " ";
            }
            std::cout << "\n";
        }

        std::cout << "\nOutput dimensions: "
                  << output.height << "x" << output.width << "\n";

        return 0;
    } else {
        std::cout << "Failed! The algorithm could not find a valid solution.\n";
        std::cout << "Try adjusting the constraints, dimensions, or seed.\n";
        return 1;
    }
}
