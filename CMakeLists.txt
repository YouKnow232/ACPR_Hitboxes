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

include(FetchContent)

set(CDT_USE_AS_COMPILED_LIBRARY OFF)
FetchContent_Declare(
    CDT
    GIT_REPOSITORY https://github.com/artem-ogre/CDT.git
    GIT_TAG 5a1d702a01b24b33555913b90934fe673d51fc0f # v1.4.4
)
set(CLIPPER2_EXAMPLES OFF)
set(CLIPPER2_TESTS OFF)
FetchContent_Declare(
    Clipper2
    GIT_REPOSITORY https://github.com/AngusJohnson/Clipper2.git
    GIT_TAG 21ebba05db8894f0c7217ad35ea518080f324946 # v2.0.1
)

FetchContent_MakeAvailable(
    CDT
    Clipper2
)

message(STATUS "clipper2 source dir: ${clipper2_SOURCE_DIR}")

add_library(Clipper2 STATIC
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/src/clipper.engine.cpp
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/src/clipper.offset.cpp
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/src/clipper.rectclip.cpp
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/src/clipper.triangulation.cpp
)
target_include_directories(Clipper2 PUBLIC
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/include
)

add_library(hitboxes SHARED
    source/source.cpp
    source/cleanHitRecorder/cleanHitRecorder.cpp
    source/logging/logging.cpp
    source/dataStore/dataStore.cpp
    source/graphics/geometry.cpp
    source/graphics/graphics.cpp
    source/graphics/render.cpp
    source/settings/modMenu.cpp
)
target_include_directories(hitboxes PRIVATE
    include
    source
    third_party
    ${cdt_SOURCE_DIR}/CDT/include
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/include
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
    Clipper2
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
