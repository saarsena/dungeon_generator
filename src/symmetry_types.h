#ifndef DUNGEON_GENERATOR_SYMMETRY_TYPES_H
#define DUNGEON_GENERATOR_SYMMETRY_TYPES_H

// Shared enumerator list for the SymmetryType enum used by both the v1
// (GDTilingWFC) and v2 (WFCConfiguration) tiling-WFC bindings.
//
// Each class keeps its own nested `enum SymmetryType` so the existing Godot
// bindings (VARIANT_ENUM_CAST / BIND_ENUM_CONSTANT) continue to work and the
// public GDScript API is preserved. Expanding this macro in the enum body
// guarantees the values stay in sync across both classes.
#define DG_SYMMETRY_TYPE_VALUES                                               \
    SYMMETRY_X = 0,         /* No rotation - 1 orientation */                 \
    SYMMETRY_I = 1,         /* 2 orientations (vertical/horizontal) */        \
    SYMMETRY_BACKSLASH = 2, /* 2 orientations (diagonal) */                   \
    SYMMETRY_T = 3,         /* 4 orientations (T-shape) */                    \
    SYMMETRY_L = 4,         /* 4 orientations (L-shape) */                    \
    SYMMETRY_P = 5          /* 8 orientations (fully asymmetric) */

#endif // DUNGEON_GENERATOR_SYMMETRY_TYPES_H
