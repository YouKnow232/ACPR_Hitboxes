cmake_minimum_required(VERSION 3.21)
project(ACPRHitboxes
    VERSION 1.0.0
    DESCRIPTION "Hitbox mod for GGXXACPR"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR})

add_library(hitboxes SHARED
    source/source.cpp
    source/logging/logging.cpp
    source/graphics/graphics.cpp
    source/graphics/render.cpp
    source/settings/settings.cpp
)
target_include_directories(hitboxes PRIVATE
    include
    source
    third_party
)
set_target_properties(hitboxes PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}/Hitboxes
    PREFIX ""
)
target_compile_options(hitboxes PRIVATE
    "-msse2"
)

target_link_libraries(hitboxes
    d3d9
    d3dcompiler
)
target_link_options(hitboxes PRIVATE
    -static
)

add_custom_command(
    OUTPUT ${BIN_DIR}/Hitboxes/config.json
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/source/config.json
        ${BIN_DIR}/Hitboxes/config.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/config.json
    COMMENT "Copying config"
)
add_custom_target(copy_config ALL
    DEPENDS ${BIN_DIR}/Hitboxes/config.json
)

add_custom_command(
    OUTPUT ${BIN_DIR}/Hitboxes/shaders/HitboxShader.hlsl
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/shaders/HitboxShader.hlsl
        ${BIN_DIR}/Hitboxes/shaders/HitboxShader.hlsl
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/graphics/shaders/HitboxShader.hlsl
    COMMENT "Copying shader"
)
add_custom_target(copy_shader ALL
    DEPENDS ${BIN_DIR}/Hitboxes/shaders/HitboxShader.hlsl
)
