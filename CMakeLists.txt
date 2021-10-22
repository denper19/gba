cmake_minimum_required(VERSION 3.0.0)

#add_compile_options(-fsanitize=undefined)
#add_link_options(-fsanitize=undefined)

project(gba VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)

include(CTest)
enable_testing()

find_package(SDL2 CONFIG REQUIRED)

set(PROJECT_SOURCES
    "src/mem.hpp"
    "src/mem.cpp" 
    "src/cpu.hpp"
    "src/cpu.cpp"
    "src/bus.hpp"
    "src/bus.cpp"
    "src/ppu.hpp"
    "src/ppu.cpp"
    "src/main.cpp"
)

add_executable(gba ${PROJECT_SOURCES})
target_link_libraries(gba PRIVATE SDL2::SDL2 SDL2::SDL2main)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
