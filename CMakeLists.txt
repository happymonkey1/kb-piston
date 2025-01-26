cmake_minimum_required(VERSION 3.15) # Specify minimum CMake version
project(kb-piston) # Set your project name

set(KB_PISTON_SRCS
    # /core/
    src/kb/piston/core/core.h
    src/kb/piston/core/types.h
    src/kb/piston/core/fixed_string.h
    # /ecs/
    src/kb/piston/ecs/components.h
    # /engine/
    src/kb/piston/engine/js_engine.h
    src/kb/piston/engine/js_engine.cpp
    # /error/
    src/kb/piston/error/js_error.h
    # /event/
    src/kb/piston/event/dispatcher.h
    src/kb/piston/event/event.h
    src/kb/piston/event/meta.h
    src/kb/piston/event/orchestrator.h
    # /log/
    src/kb/piston/log/logger.h
    src/kb/piston/log/logger.cpp
    # /meta/
    src/kb/piston/meta/meta.h
    # /runtime/
    src/kb/piston/runtime/application.h
    src/kb/piston/runtime/application.cpp
    src/kb/piston/runtime/console.h
    src/kb/piston/runtime/console.cpp
    # /script/
    src/kb/piston/script/js_script_error.h
    src/kb/piston/script/js_script.h
    src/kb/piston/script/js_script.cpp
    # /util/
    src/kb/piston/util/file_util.h
)
set(KB_PISTON_EXE kb-piston)
set(KB_PISTON_LIB kb-piston-lib)

set(CMAKE_CXX_STANDARD 20)

set(
    V8_LIB_PATH
    ${CMAKE_SOURCE_DIR}/vendor/v8/out/x64.static-release/obj
)

if (WIN32)
    set(
        V8_MONOLITH_LIB_PATH
        ${V8_LIB_PATH}/obj/v8_monolith.lib
    )

    set(
        V8_BUILD_SCRIPT
        ${CMAKE_SOURCE_DIR}/scripts/windows/build-v8.bat
    )
endif (WIN32)
if (UNIX)
    # TODO: fix file name / extension
    set(
        V8_MONOLITH_LIB_PATH
        ${V8_MONOLITH_LIB_PATH}/obj/v8_monolith.a
    )

    set(
        V8_BUILD_SCRIPT
        ./${CMAKE_SOURCE_DIR}/scripts/linux/build-v8.sh
    )
endif (UNIX)

# Build V8
add_custom_target(BUILD_V8
    COMMAND ${V8_BUILD_SCRIPT}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vendor/v8"
)

add_library(V8 STATIC IMPORTED)
#set_target_properties(V8 PROPERTIES
#    INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/vendor/v8/include"
#    IMPORTED_LOCATION ${V8_MONOLITH_LIB_PATH}
#    LINKER_LANGUAGE CXX
#)

# Use static release runtime, as that is what V8 is compiled with
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

link_directories(V8_LIB_PATH)

set(FMTLIB_SRCS
    "vendor/fmtlog/fmt/include/fmt/core.h"
    "vendor/fmtlog/fmt/include/fmt/format.h"
    "vendor/fmtlog/fmt/include/fmt/format-inl.h"
    "vendor/fmtlog/fmt/src/format.cc"
)

# Add library
add_library(${KB_PISTON_LIB} STATIC ${KB_PISTON_SRCS} ${FMTLIB_SRCS})
target_link_libraries(${KB_PISTON_LIB} PUBLIC
    ${V8_LIB_PATH}/v8_monolith.lib
    WinMM
    dbghelp # ref: https://stackoverflow.com/a/76240854
)
target_compile_definitions(${KB_PISTON_LIB} PRIVATE
    -DV8_COMPRESS_POINTERS=1
    -DV8_31BIT_SMIS_ON_64BIT_ARCH=1
    -DV8_ENABLE_SANDBOX=1
    $<$<CONFIG:Debug>:KB_DEBUG>
    $<$<CONFIG:Release>:KB_RELEASE>
)

# Include kb-piston directories
include_directories(${KB_PISTON_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${KB_PISTON_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/v8/include)
include_directories(${KB_PISTON_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmtlog)
include_directories(${KB_PISTON_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/fmtlog/fmt/include)
include_directories(${KB_PISTON_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/include)

# Add a test executable
add_executable(${KB_PISTON_EXE}
    src/kb/piston/entrypoint.cpp)
target_link_libraries(${KB_PISTON_EXE} PUBLIC
    ${KB_PISTON_LIB}
)
target_compile_definitions(${KB_PISTON_EXE} PRIVATE
    -DV8_COMPRESS_POINTERS=1
    -DV8_31BIT_SMIS_ON_64BIT_ARCH=1
    -DV8_ENABLE_SANDBOX=1
    $<$<CONFIG:Debug>:KB_DEBUG>
    $<$<CONFIG:Release>:KB_RELEASE>
)
include_directories(${KB_PISTON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${KB_PISTON_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/include)

# Set output directory
set(OPERATING_SYSTEM $<IF:$<PLATFORM_ID:Windows>,windows,linux>)
set(ARCHITECTURE $<IF:$<BOOL:${CMAKE_SIZEOF_VOID_P}>,x86_x64,x86>)
set(RELEASE $<CONFIG>)

# Specify the custom output directory structure
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/${RELEASE}-${OPERATING_SYSTEM}-${ARCHITECTURE}")

set_target_properties(${KB_PISTON_LIB} ${KB_PISTON_EXE}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}/${PROJECT_NAME}"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}/${PROJECT_NAME}"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}/${PROJECT_NAME}"
)

# Configure output directory for each release type
foreach(CONFIG_TYPE Debug Release RelWithDebInfo MinSizeRel)
    set_target_properties(${KB_PISTON_LIB} ${KB_PISTON_EXE}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} "${OUTPUT_DIR}/${PROJECT_NAME}"
        LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE} "${OUTPUT_DIR}/${PROJECT_NAME}"
        ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE} "${OUTPUT_DIR}/${PROJECT_NAME}"
    )
endforeach()

#set_property(TARGET ${KB_PISTON_EXE} PROPERTY
#    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


#set_property(TARGET ${KB_PISTON_LIB} PROPERTY
#    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (WIN32)

endif (WIN32)

# ref: https://stackoverflow.com/a/60890947
if (MSVC)
    target_compile_options(${KB_PISTON_EXE} PUBLIC "/Zc:__cplusplus")
    target_compile_options(${KB_PISTON_LIB} PUBLIC "/Zc:__cplusplus")
endif()
