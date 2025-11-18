# PocketBeagle ↔ Linux Streaming Helper

The optional PB2 build produces two executables that let a PocketBeagle render
a NES ROM headlessly and stream video/input to a Linux desktop over the
network:

- `pb2` runs on the PocketBeagle. It announces the `_haznes._tcp` DNS-SD
  service, accepts ROM uploads over TCP, streams 256×240 frames over UDP, and
  feeds controller bytes back into the emulator.
- `linux_render` runs on a Linux workstation/RPi. It discovers the PB2 via
  mDNS, uploads a ROM, sends controller input (keyboard by default), and
  renders the streamed frames via SDL2.

## Requirements

- CMake 3.16+ and a standard C toolchain on both sides.
- SDL2 is fetched automatically for the Linux renderer, so no extra packages.
- The [`mdns`](https://github.com/mjansson/mdns) library is pulled in via
  `FetchContent`. If you are missing CMake, install it with:

  ```bash
  sudo apt install cmake
  ```

- Ensure both devices share the same network segment (PB2 usually exposes
  itself as `usb0`, the host can use any interface with multicast enabled).

## Build

From the project root:

```bash
cmake -S . -B build -D BUILD_PB2_MDNS_EXAMPLES=ON
cmake --build build
```

This produces `build/pb2` (ARM binary) and `build/linux_render` (x86/ARM host).

## Deploying to the PocketBeagle

Copy the PB2 executable to the board (replace host/user as needed):

```bash
scp build/pb2 debian@pb2.local:/home/debian/pb2
```

SSH in and launch it:

```bash
ssh debian@pb2.local
./pb2
```

The program prints its chosen IP + UDP port, then waits for a client.

## Running the Linux Renderer

On your desktop/RPi:

```bash
./build/linux_render [optional/path/to/game.nes]
```

Steps performed automatically:

1. Discover `_haznes._tcp` via mDNS and connect to the PB2’s TCP port.
2. Upload the ROM (if a path was supplied).
3. Open a local UDP socket, send the port to the PB2, and start streaming.
4. Render incoming frames via SDL2 and forward keyboard input back to PB2.

If you omit the ROM argument, the PB2 keeps the last uploaded game.

## Notes

- The mDNS service name, TXT metadata, and packet format are documented in
  `PB2.h`. If you change the service name, update both binaries.
- You can record TAS-style controller logs by capturing the single input byte
  sent per frame.
- When debugging connectivity use `journalctl`/`dmesg` on PB2 to confirm the
  interface (`usb0`, `eth1`, etc.) is up and has multicast enabled.
