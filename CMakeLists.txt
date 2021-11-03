cmake_minimum_required(VERSION 3.0.0)
project(gba VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)

include(CTest)
enable_testing()

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
#find_package(Gperftools REQUIRED) 

set(PROJECT_SOURCES
    "src/imgui_memory_editor.h"
    "src/utils.cpp"
    "src/utils.hpp"
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
target_link_libraries(gba PRIVATE ImGui-SFML::ImGui-SFML)
target_link_libraries(gba PRIVATE imgui::imgui)
target_link_libraries(gba PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

#find_package(CAPSTONE REQUIRED capstone)

# Use CAPSTONE_FOUND, CAPSTONE_LIBRARIES, CAPSTONE_INCLUDE_DIRS vars
#target_link_libraries(${PROJECT_NAME} ${CAPSTONE_LIBRARIES})


#target_link_libraries(gba ${GPERFTOOLS_TCMALLOC})
#target_link_libraries(gba ${GPERFTOOLS_PROFILER})
#target_link_libraries(gba ${GPERFTOOLS_TCMALLOC_AND_PROFILER})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
