cmake_minimum_required(VERSION 3.9)
project(DungeonGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Deploy directory - everything goes here for easy copying
set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")

# Platform-specific settings
if(WIN32)
    set(TARGET_PATH ${DEPLOY_DIR}/addons/dungeon_generator/bin)
    set(LIB_SUFFIX ".windows.template_debug.x86_64")
    set(LIB_SUFFIX_RELEASE ".windows.template_release.x86_64")
    set(LIB_EXT ".dll")
elseif(UNIX AND NOT APPLE)
    set(TARGET_PATH ${DEPLOY_DIR}/addons/dungeon_generator/bin/linux)
    set(LIB_SUFFIX ".linux.template_debug.x86_64")
    set(LIB_SUFFIX_RELEASE ".linux.template_release.x86_64")
    set(LIB_EXT ".so")
elseif(APPLE)
    set(TARGET_PATH ${DEPLOY_DIR}/addons/dungeon_generator/bin/macos)
    set(LIB_SUFFIX ".macos.template_debug.universal")
    set(LIB_SUFFIX_RELEASE ".macos.template_release.universal")
    set(LIB_EXT ".dylib")
endif()

# Enable position independent code for static libraries (required for linking into shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add tiling-wfc library
add_subdirectory(tiling-wfc)

# Enable exceptions for godot-cpp (needed for WFC error handling)
set(GODOTCPP_DISABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)

# Disable static linking for Linux (use dynamic linking instead)
set(GODOTCPP_USE_STATIC_CPP OFF CACHE BOOL "" FORCE)

# Add godot-cpp
add_subdirectory(godot-cpp)

# Create GDExtension library
add_library(${PROJECT_NAME} SHARED
    # Original v1 files (keep for backward compatibility)
    src/gdwfc.cpp
    src/gdwfc.h

    # New v2 files
    src/gdwfc_v2.cpp
    src/gdwfc_v2.h

    # Walker dungeon generation
    src/walker.cpp
    src/walker.h

    # BSP dungeon generation
    src/bsp_godot.cpp
    src/bsp_godot.h

    # Overlapping WFC generation
    src/overlapping_wfc_godot.cpp
    src/overlapping_wfc_godot.h

    # Fast-WFC library source (for Overlapping WFC)
    fast-wfc/src/lib/wfc.cpp

    # Unified registration (includes both v1 and v2)
    src/register_types.cpp
    src/register_types.h
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    godot-cpp
    tiling_wfc_static
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    tiling-wfc/include/
    fast-wfc/src/include/
)

# Set output directory and naming
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME_DEBUG "${PROJECT_NAME}${LIB_SUFFIX}"
    OUTPUT_NAME_RELEASE "${PROJECT_NAME}${LIB_SUFFIX_RELEASE}"
    SUFFIX "${LIB_EXT}"
    # For multi-config generators (Visual Studio), put outputs in deploy directory
    LIBRARY_OUTPUT_DIRECTORY "${TARGET_PATH}"
    RUNTIME_OUTPUT_DIRECTORY "${TARGET_PATH}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${TARGET_PATH}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${TARGET_PATH}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${TARGET_PATH}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${TARGET_PATH}"
)

# Copy addon files to deploy directory for easy distribution
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}/addons/dungeon_generator"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/addons/dungeon_generator/dungeon_generator.gdextension"
        "${DEPLOY_DIR}/addons/dungeon_generator/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/addons/dungeon_generator/plugin.cfg"
        "${DEPLOY_DIR}/addons/dungeon_generator/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/addons/dungeon_generator/plugin.gd"
        "${DEPLOY_DIR}/addons/dungeon_generator/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/addons/dungeon_generator/DungeonPreview.gd"
        "${DEPLOY_DIR}/addons/dungeon_generator/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/addons/dungeon_generator/examples"
        "${DEPLOY_DIR}/addons/dungeon_generator/examples"
    COMMENT "Copying addon files to deploy directory"
)

# Print helpful message
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Build complete! Copy to your project:"
    COMMAND ${CMAKE_COMMAND} -E echo "  FROM: ${DEPLOY_DIR}/addons/dungeon_generator"
    COMMAND ${CMAKE_COMMAND} -E echo "  TO:   /addons/dungeon_generator"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
)

# Platform-specific compiler flags
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /EHsc)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -fexceptions)
endif()
