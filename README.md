# HazardousNES

A work‑in‑progress Nintendo Entertainment System emulator written in C, using SDL2 for graphics and input. The project focuses on correctness-first CPU/PPU emulation, a simple UI, and a clean CMake build that fetches SDL2 automatically.

This README covers how to build and run it, what currently works, and known limitations,for the emulator.

## Status

- CPU/PPU core running with background + sprite rendering
- Controller 1 (keyboard) supported
- iNES/NES 2.0 headers parsed
- Mappers: NROM (0), MMC1 (1) UxROM (2), CNROM (3) implemented.
- Nes file select
  - Windows: File → Open ROM menu
  - Linux: ctrl + o

Limitations:

- Audio (APU) not implemented yet
- Only 1 controller mapped; JOY2 is disabled
- Mapper coverage is limited (MMC3/5 not yet implemented)
- Mac file select has not been implemented
- F1 Race (It seems it is a game that breaks on alot on many Emulators)

## Build

Prerequisites:

- CMake 3.16+
- A C compiler and build tools
  - Windows: Visual Studio 2022 (MSVC)
  - Linux: GCC/Clang + build‑essentials
- Internet access (CMake will FetchContent SDL2 automatically)

Clone and build (Release):

```bash
git clone https://github.com/pedrocriado/NES-EMULATOR.git
cd NES-EMULATOR
cmake -S . -B build
cmake --build build
```

SDL2 is fetched and built as part of the project; you don’t need to install it system‑wide.

### PocketBeagle ↔ Linux streaming helper

Enable the `BUILD_PB2` CMake flag to build two extra binaries:

- `pb2` — an mDNS-advertised PocketBeagle daemon that accepts ROM uploads over
  TCP, streams the emulator framebuffer over UDP, and ingests controller bytes.
- `linux_render` — a Linux desktop client that discovers the PB2 via DNS-SD,
  uploads a ROM (optional), renders the streamed frames via SDL2, and forwards
  keyboard input back to the board.

Build them with:

```bash
cmake -S . -B build -D BUILD_PB2=ON
cmake --build build
```

Deployment and usage instructions live in `src/Pocket_Beagle_2/PB2_README.md`.
That document covers copying the `pb2` binary to the board, launching both
programs, and the packet format if you want to extend the streaming protocol.

## Run

You can select a rom by pressing ctrl + O. Windows systems has a drop file select button at the top of the window. Along side this options you can also run the emualtor by passing the rom file location.

Windows
```bash
./build/Debug/nes ./path/to/file/file.nes
```

Linux
```bash
./build/nes ./path/to/file/file.nes
```

ROMs are searched relative to `nes` executable location.

## Controls

- D‑Pad: Arrow keys
- A: `Z`
- B: `X`
- Select: `G`
- Start: `F`

Note: Only controller 1 is currently mapped.

## Compatibility

- File formats: iNES and NES 2.0 headers
- Mappers:
  - Implemented: NROM (0), MMC1 (1), UxROM (2), CNROM (3)
  - Plan to Implement: MMC3 (4), MMC5 (5)

The addition of these mappers would allow for many more games to be played on the emulator.

## Screenshots

Below are some screenshots taken from the games running on the emulator:

_Super Mario Bros._
![Super Mario Bros. – Title](docs/screenshots/smb_title_screen.png)

_Castlevania_
![Castlevania](docs/screenshots/Castlevania.png)

## Project Layout

- `src/Core/` – Emulator sources (CPU6502, PPU, Bus, Cartridge, Controller).
- `src/Core/Mappers/` – implemented mappers.
- `src/Desktop/` – implementation of desktop application.
- `src/Pocket_Beagle_2` – Port Emulator to the Pocket Beagle 2.
- `nes_files/` – Place your ROMs here (ignored in .git); saves in `nes_files/saves/`.
- `CMakeLists.txt` – Build configuration.

## Legal

This project does not provide or endorse distribution of copyrighted ROMs. Use your own legally obtained ROM backups. Some public test ROMs are widely available for verification and debugging.

## License

- Project license: MIT — see `LICENSE`.
- SDL2 is used under the zlib license (via FetchContent). If you redistribute static binaries, ensure you retain SDL2’s license notice. See: https://libsdl.org/license.php
