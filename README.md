# Ruby RemotePlay for Windows

Ruby RemotePlay is a Windows-focused PlayStation Remote Play client built from the actively maintained [Chiaki-ng](https://github.com/chiaki-ng/chiaki-ng) engine. It is intended for PS4 and PS5 owners who want a native desktop client with console discovery, pairing, PSN registration, streaming, controller mapping, keyboard input, wake/sleep, and local or remote-network connections.

> This project is not endorsed, certified, or otherwise affiliated with Sony Interactive Entertainment LLC. PlayStation, PS4, PS5, DualSense, and DUALSHOCK are trademarks of Sony Interactive Entertainment.

## Current state

The repository contains the full open-source Remote Play implementation and the Windows build workflows needed to produce an installable client. The Ruby RemotePlay name is a Windows-facing rebrand; the streaming and registration engine remains based on Chiaki-ng so the project can use its mature PS4/PS5 protocol implementation instead of duplicating a proprietary protocol from scratch.

## Build on Windows

The supported build path is MSYS2. The project already includes Windows CI workflows under `.github/workflows/` and the deployment script at `scripts/deploy-windows-msys2.sh`.

At a high level:

1. Install the MSYS2 toolchain and Qt 6 dependencies described in [`doc/platform-build.md`](doc/platform-build.md).
2. Clone this repository with submodules:

   ```bash
   git clone --recurse-submodules https://github.com/OGBlizzard/Ruby-RemotePlay.git
   cd Ruby-RemotePlay
   ```

3. Configure and build with CMake in the MSYS2 environment. The existing Windows workflow is the reference command sequence: [`build-msys2.yml`](.github/workflows/build-msys2.yml).
4. Run the generated `chiaki.exe` from the packaged output, or use the Windows installer produced by the deployment step.

A Windows build cannot be produced in this Linux sandbox because the project requires the Windows/Qt/MSYS2 toolchain and multimedia dependencies. The repository is ready for GitHub Actions to build it on `windows-latest`.

## First connection

Enable Remote Play on the console, put the console in registration mode, discover it from the app or add its IP address manually, then register it with the displayed PIN. A PlayStation Network account ID is required for modern PS4/PS5 registration. Use a DualSense, DualSense Edge, or DUALSHOCK 4 controller over USB on Windows.

For a stable experience, Sony recommends at least 5 Mbps broadband and 15 Mbps for a better Remote Play experience. The console and PC should use a reliable network connection.

## Custom scripts

Open **Settings → Config → Custom Scripts** to create, import, enable, disable, run, or delete JavaScript automation. Scripts can react to application start, connection attempts, session start, and disconnect events through a small local API for logging and notifications. They do not receive shell, filesystem, network, controller, or credential APIs. See [`docs/custom-scripts.md`](docs/custom-scripts.md) and the ready-to-import [`examples/session-notifier.js`](examples/session-notifier.js).

## License and attribution

Ruby RemotePlay is distributed under the GNU Affero General Public License v3.0. See [`COPYING`](COPYING). It includes and modifies code from Chiaki-ng and retains the upstream notices and third-party licenses. If you distribute binaries or a network-served modified version, provide the corresponding source as required by the AGPL.
