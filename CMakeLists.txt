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
    src/Controller.c
    src/Graphics.c
    src/Mappers/Mapper0.c
    src/Mappers/Mapper1.c
    src/Mappers/Mapper2.c
    src/Mappers/Mapper3.c
    src/Mappers/Mapper4.c
    src/Mappers/Mapper5.c
)

add_executable(nes ${SOURCES})

# SDL2 via FetchContent
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.28.5
)
FetchContent_MakeAvailable(SDL2)

# Link SDL2 targets
target_link_libraries(nes PRIVATE SDL2::SDL2 SDL2::SDL2main)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GTK3 gtk+-3.0)
    endif()

    if(GTK3_FOUND)
        target_compile_definitions(nes PRIVATE GRAPHICS_HAS_GTK=1)
        target_include_directories(nes PRIVATE ${GTK3_INCLUDE_DIRS})
        target_link_directories(nes PRIVATE ${GTK3_LIBRARY_DIRS})
        target_link_libraries(nes PRIVATE ${GTK3_LIBRARIES})
        target_compile_options(nes PRIVATE ${GTK3_CFLAGS_OTHER})
        target_link_options(nes PRIVATE ${GTK3_LDFLAGS_OTHER})
    endif()
endif()

if(WIN32)
    target_link_libraries(nes PRIVATE comdlg32 winmm)
endif()
