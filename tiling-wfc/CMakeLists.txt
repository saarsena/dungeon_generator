cmake_minimum_required(VERSION 3.9)
project(tiling_wfc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(DEFAULT_BUILD_TYPE "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
endif()

include(GNUInstallDirs)

# Source files for the library
set(SOURCE_FILES
    src/wave.cpp
    src/propagator.cpp
    src/wfc.cpp
)

# Header files (for installation)
set(HEADER_FILES
    include/tiling_wfc.hpp
    include/wfc.hpp
    include/propagator.hpp
    include/wave.hpp
    include/direction.hpp
    include/utils/array2D.hpp
    include/utils/array3D.hpp
)

# Create static library
add_library(${PROJECT_NAME}_static STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME}_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(${PROJECT_NAME}_static PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Optional: Build example
option(BUILD_EXAMPLE "Build example application" ON)
if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# Installation
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static
    EXPORT TilingWFCTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT TilingWFCTargets
    FILE TilingWFCTargets.cmake
    NAMESPACE TilingWFC::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TilingWFCConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/TilingWFCConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/TilingWFCConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TilingWFCConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TilingWFCConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
