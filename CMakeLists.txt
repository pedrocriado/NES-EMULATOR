cmake_minimum_required(VERSION 3.16)
project(nes_emulator C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)

# Source files
set(SOURCES
    src/main.c
    src/CPU6502.c
    src/PPU.c
    src/APU.c
    src/Bus.c
    src/NES.c
    src/Cartridge.c
)

add_executable(nes src/main.c)

# SDL2 via FetchContent
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.28.5
)
FetchContent_MakeAvailable(SDL2)

# Link SDL2 targets
target_link_libraries(nes PRIVATE SDL2::SDL2-static SDL2::SDL2main)

