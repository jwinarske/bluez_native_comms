cmake_minimum_required(VERSION 3.22)
project(bluez_nc CXX C)

set(CMAKE_CXX_STANDARD  23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Header-only type library (for now) ───────────────────────────────────────
add_library(bluez_types INTERFACE)
target_include_directories(bluez_types INTERFACE include)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
