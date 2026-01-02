# Wave Function Collapse GDExtension

This addon provides the `GDTilingWFC` class for procedural generation using the Wave Function Collapse algorithm.

## Installation

**IMPORTANT:** You must build the extension binaries before using this addon!

### Build Instructions

1. From the project root (`godot-wfc/`), run:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

2. The compiled binaries will be placed in `addons/wfc/bin/`

3. Copy the entire `addons/wfc/` folder to your Godot project's `addons/` directory

4. Restart Godot or reload your project

5. Enable the plugin in Project > Project Settings > Plugins

## Usage

Once enabled, the `GDTilingWFC` class will be available in GDScript. See the main project README and `example_wfc.gd` for usage examples.

## Quick Example

```gdscript
var wfc = GDTilingWFC.new()
wfc.set_size(20, 20)
wfc.set_seed(12345)
# ... add tiles and rules ...
var result = wfc.run()
```
