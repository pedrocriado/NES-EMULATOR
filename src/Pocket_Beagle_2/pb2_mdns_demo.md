# PocketBeagle ↔ Linux mDNS TCP Demo

This repository now ships with two tiny helper programs that demonstrate how a
PocketBeagle (PB2) can advertise itself via mDNS/Bonjour and exchange TCP data
with a Linux host (RPi, desktop, SBC, etc.) using plain BSD sockets.

- `pb2_mdns_server` — runs on the PocketBeagle. It registers the
  `_haznes._tcp` mDNS service, waits for a connection on TCP port `49733`,
  prints controller payloads, and responds with example frame counters.
- `linux_mdns_client` — runs on any modern Linux host (RPi, desktop, etc.). It browses
  for the `_haznes._tcp` service, connects to the PB2, sends a handful of
  controller strings, and prints the replies.

## Dependencies

No extra system packages are required. The optional build flag pulls the
[`mdns`](https://github.com/mjansson/mdns) single-header mDNS/DNS-SD library via
CMake’s `FetchContent`, so a standard toolchain (`cmake`, compiler, make) is all
you need on both boards. You should run this command to install it:
```bash
sudo apt isntall cmake
```

## Building the samples

Enable the optional CMake toggle when configuring the project (disabled by
defaul):

```bash
cmake -S . -B build -D BUILD_PB2_MDNS_EXAMPLES=ON
cmake --build build
```

This produces two executables in the build folder `pb2_mdns_server` and
`linux_mdns_client`. We will need run `pb2_mdns_server` on the pocket beagle 2. To do this will need to send the executable by running the command:
```bash
scp /path/to/file <name>@<ip>:/path/to/file
```
Now that the executable is on the Pocket Beagle 2 all you have to do is run it. When the PB2 server executable is running you can use the Linux client executable on your system to run and communicate with the system.

TODO: add a file send function to allow the Pocket beagle to change from one game to another.
