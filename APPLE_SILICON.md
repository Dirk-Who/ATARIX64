# AtariX on Apple Silicon

Maintained by Dirk Werth with development assistance from OpenAI Codex.

This branch is derived from Andreas Kromke's original AtariX and Thorsten
Otto's continuation of it. See [README.md](README.md) for the complete project
lineage, attribution and licence information.

This branch removes the remaining 32-bit/Intel assumptions from AtariX and
builds the application as native `arm64` code on macOS 11 or later.

## Requirements

- Apple Silicon Mac
- Xcode 16 or newer, including the command-line tools
- Internet access once to download the official SDL 2 framework

## Build

From the repository root:

```sh
./scripts/bootstrap-sdl2.sh
xcodebuild \
  -project src/AtariX-MT/AtariX/AtariX.xcodeproj \
  -scheme AtariX-Application \
  -configuration Release \
  -derivedDataPath build \
  ARCHS=arm64 \
  ONLY_ACTIVE_ARCH=NO \
  build
```

The application is created at:

```text
build/Build/Products/Release/AtariX.app
```

Verify that both the executable and bundled SDL framework contain native
Apple Silicon code:

```sh
lipo -info build/Build/Products/Release/AtariX.app/Contents/MacOS/AtariX
lipo -info build/Build/Products/Release/AtariX.app/Contents/Frameworks/SDL2.framework/SDL2
```

## What changed

- Carbon Multiprocessing Services were replaced with C++17 threads,
  condition variables, and recursive mutexes.
- The obsolete Carbon `Point` dependency was replaced with AtariX's own
  16-bit point type.
- 68k callback parameters are translated through the emulated RAM base
  instead of being treated as truncated host pointers.
- Unused legacy 32-bit PowerPC host-address fields are no longer populated.
- Atari ABI structures use portable one-byte packing pragmas.
- The Xcode target now builds `arm64` and `x86_64` with a macOS 11 deployment
  target.
- The Intel-only bundled SDL 2 framework can be replaced reproducibly with
  the official universal SDL 2.32.10 framework.
- The current 0.6.6 release supports resizable, HiDPI-aware and native
  fullscreen output with aspect-ratio-preserving scaling and corrected mouse
  coordinates. Fullscreen mouse motion is coalesced for smooth input without
  reducing emulation performance. Mouse wheels and MacBook trackpads generate
  Atari-compatible scroll events.
- Native file-descriptor-to-drive state is kept outside emulated RAM so
  writable mapped drives work reliably on 64-bit hosts. File permission and
  ownership calls use macOS-compatible success semantics. Optional legacy
  Finder Type/Creator metadata and drive-sync requests are handled compatibly
  so Jinnee keeps copied files.
- ARM64 framebuffer byte-lane handling and 32-bit RGB/VDI colour conversion
  have been corrected.
- MagiC guest power-off requests now close AtariX cleanly.

## Current validation limit

The source-level and platform changes can be checked on any system, but the
final application must be compiled and launched on macOS because it depends
on AppKit, Core Foundation, and the macOS SDL framework. The GitHub Actions
workflow builds the `arm64` application, ad-hoc signs the app and bundled SDL2
framework, and performs strict signature verification. The result is not
notarised with an Apple Developer ID.
