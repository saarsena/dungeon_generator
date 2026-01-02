#include "register_types.h"

#include "gdwfc.h"
#include "gdwfc_v2.h"
#include "walker.h"
#include "bsp_godot.h"
#include "overlapping_wfc_godot.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdwfc_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    UtilityFunctions::print("GDTilingWFC Extension v1.0.1 - Initializing...");

    // Register v1 class
    ClassDB::register_class<GDTilingWFC>();

    // Register v2 classes
    ClassDB::register_class<WFCResult>();
    ClassDB::register_class<WFCConfiguration>();
    ClassDB::register_class<GDTilingWFCv2>();

    // Register Walker classes
    ClassDB::register_class<WalkerResult>();
    ClassDB::register_class<WalkerDungeonGenerator>();

    // Register BSP classes
    ClassDB::register_class<BSPResult>();
    ClassDB::register_class<BSPDungeonGenerator>();

    // Register Overlapping WFC classes
    ClassDB::register_class<OverlappingWFCResult>();
    ClassDB::register_class<OverlappingWFCGenerator>();

    UtilityFunctions::print("GDTilingWFC: Registered WalkerDungeonGenerator, BSPDungeonGenerator, Overlapping WFC, and Tiling WFC classes");
}

void uninitialize_gdwfc_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT gdwfc_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_gdwfc_module);
    init_obj.register_terminator(uninitialize_gdwfc_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
