cmake_minimum_required(VERSION 3.20)

project(Hesan2D VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(NOT HESAN_RAYLIB_ROOT)
    if(DEFINED ENV{RAYLIB_ROOT})
        set(HESAN_RAYLIB_ROOT "$ENV{RAYLIB_ROOT}")
    elseif(EXISTS "C:/msys64/mingw64/include/raylib.h")
        set(HESAN_RAYLIB_ROOT "C:/msys64/mingw64")
    endif()
endif()

if(HESAN_RAYLIB_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${HESAN_RAYLIB_ROOT}")
endif()

set(raylib_USE_STATIC_LIBS OFF CACHE BOOL "" FORCE)
find_package(raylib REQUIRED)

message(STATUS "raylib: ${raylib_INCLUDE_DIRS} | ${raylib_LIBRARIES}")

include(FetchContent)

set(TMXLITE_STATIC_LIB ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    tmxlite
    GIT_REPOSITORY https://github.com/fallahn/tmxlite.git
    GIT_TAG v1.4.5
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR tmxlite
)
FetchContent_MakeAvailable(tmxlite)

file(GLOB_RECURSE HESAN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

if(NOT HESAN_SOURCES)
    message(FATAL_ERROR "No source files found under ${CMAKE_CURRENT_SOURCE_DIR}/src")
endif()

add_executable(Hesan2D ${HESAN_SOURCES})

target_include_directories(Hesan2D PRIVATE src)
target_link_libraries(Hesan2D PRIVATE raylib tmxlite)

get_target_property(TMXLITE_INCLUDES tmxlite INTERFACE_INCLUDE_DIRECTORIES)
if(TMXLITE_INCLUDES)
    target_include_directories(Hesan2D SYSTEM PRIVATE ${TMXLITE_INCLUDES})
endif()

if(MSVC)
    target_compile_options(Hesan2D PRIVATE /W4 /permissive-)
else()
    target_compile_options(Hesan2D PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif()

if(WIN32 AND HESAN_RAYLIB_ROOT AND EXISTS "${HESAN_RAYLIB_ROOT}/bin/libraylib.dll")
    add_custom_command(TARGET Hesan2D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${HESAN_RAYLIB_ROOT}/bin/libraylib.dll"
                "$<TARGET_FILE_DIR:Hesan2D>"
        COMMENT "Copying libraylib.dll next to Hesan2D"
    )
endif()
