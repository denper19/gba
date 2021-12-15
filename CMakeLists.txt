cmake_minimum_required(VERSION 3.0.0)

#add_compile_options(-fsanitize=undefined)
#add_link_options(-fsanitize=undefined)

project(gba VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)

find_package(SDL2 CONFIG REQUIRED)

find_library(guiLib NAMES capstone capstone_dll PATH "C:\\Users\\Laxmi\\OneDrive\\Desktop\\cap")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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

add_executable(gba ${PROJECT_SOURCES} "src/dma.cpp" "src/dma.hpp" "src/tmr.hpp" "src/tmr.cpp")

target_link_libraries(gba PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(gba PRIVATE ${guiLib})