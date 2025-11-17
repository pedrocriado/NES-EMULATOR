cmake_minimum_required(VERSION 3.16)
project(nes_emulator C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)

option(BUILD_PB2_MDNS_EXAMPLES "Build the PocketBeagle/RPi5 mDNS socket samples" OFF)

# Shared core sources
set(CORE_SOURCES
    src/Core/CPU6502.c
    src/Core/PPU.c
    src/Core/APU.c
    src/Core/Cartridge.c
    src/Core/Controller.c
    src/Core/Mappers/Mapper0.c
    src/Core/Mappers/Mapper1.c
    src/Core/Mappers/Mapper2.c
    src/Core/Mappers/Mapper3.c
    src/Core/Mappers/Mapper4.c
    src/Core/Mappers/Mapper5.c
)

add_executable(nes
    src/Desktop/NES.c
    src/Desktop/Graphics.c
    ${CORE_SOURCES}
)

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

if(BUILD_PB2_MDNS_EXAMPLES)
    if(NOT UNIX)
        message(FATAL_ERROR "The PB2 mDNS samples currently support POSIX platforms only.")
    endif()

    set(MDNS_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        mdns
        GIT_REPOSITORY https://github.com/mjansson/mdns.git
        GIT_TAG 1.4.3
    )
    FetchContent_MakeAvailable(mdns)

    find_package(Threads REQUIRED)

    add_executable(pb2
        src/Pocket_Beagle_2/PB2.c
        ${CORE_SOURCES})
    target_include_directories(pb2 PRIVATE src/Core)
    set(PB2_OPT_FLAGS -O3)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7" OR CMAKE_SYSTEM_PROCESSOR MATCHES "armv6")
        list(APPEND PB2_OPT_FLAGS -mfpu=neon)
    endif()
    target_compile_options(pb2 PRIVATE ${PB2_OPT_FLAGS})
    target_link_libraries(pb2 PRIVATE mdns::mdns Threads::Threads)

    add_executable(linux_render
        src/Pocket_Beagle_2/LinuxRender.c
        src/Desktop/Graphics.c
        ${CORE_SOURCES})
    target_include_directories(linux_render PRIVATE src/Desktop src/Core)
    target_link_libraries(linux_render PRIVATE mdns::mdns SDL2::SDL2 SDL2::SDL2main)
endif()

if(WIN32)
    target_link_libraries(nes PRIVATE comdlg32 winmm)
endif()
