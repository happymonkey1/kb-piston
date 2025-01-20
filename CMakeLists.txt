cmake_minimum_required(VERSION 3.15) # Specify minimum CMake version
project(kb-piston) # Set your project name

set(KB_PISTON_SRCS src/kb/piston/entrypoint.cpp)
set(KB_PISTON_EXE kb-piston)
set(KB_PISTON_LIB kb-piston-lib)

set(CMAKE_CXX_STANDARD 20)

# Include kb-piston directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/v8/include)

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

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

link_directories(V8_LIB_PATH)

# Add an executable
add_executable(${KB_PISTON_EXE} ${KB_PISTON_SRCS})
target_link_libraries(${KB_PISTON_EXE} PUBLIC
    ${V8_LIB_PATH}/v8_monolith.lib
    #${V8_LIB_PATH}/v8_libplatform.lib
    #${V8_LIB_PATH}/v8_libbase.lib
    WinMM
    dbghelp # ref: https://stackoverflow.com/a/76240854
)
target_compile_definitions(${KB_PISTON_EXE} PRIVATE
    -DV8_COMPRESS_POINTERS=1
    -V8_31BIT_SMIS_ON_64BIT_ARCH=1
    -DV8_ENABLE_SANDBOX=1
)

#set_property(TARGET ${KB_PISTON_EXE} PROPERTY
#    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Add library
add_library(${KB_PISTON_LIB} STATIC ${KB_PISTON_SRCS})
target_link_libraries(${KB_PISTON_LIB} PUBLIC
    ${V8_LIB_PATH}/v8_monolith.lib
    #${V8_LIB_PATH}/v8_libplatform.lib
    #${V8_LIB_PATH}/v8_libbase.lib
    WinMM
    dbghelp # ref: https://stackoverflow.com/a/76240854
)
target_compile_definitions(${KB_PISTON_LIB} PRIVATE
    -DV8_COMPRESS_POINTERS=1
    -V8_31BIT_SMIS_ON_64BIT_ARCH=1
    -DV8_ENABLE_SANDBOX=1
)
#set_property(TARGET ${KB_PISTON_LIB} PROPERTY
#    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (WIN32)

endif (WIN32)

# ref: https://stackoverflow.com/a/60890947
if (MSVC)
    target_compile_options(${KB_PISTON_EXE} PUBLIC "/Zc:__cplusplus")
    target_compile_options(${KB_PISTON_LIB} PUBLIC "/Zc:__cplusplus")
endif()
