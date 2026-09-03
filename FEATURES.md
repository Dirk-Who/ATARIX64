# AtariX 0.6.6 feature overview

This document describes the current state of the Apple Silicon continuation.
It distinguishes working features from compatibility boundaries and future
work.

## Platform and build

- Native `arm64` application for Apple Silicon
- macOS 11 or newer deployment target
- Xcode 16-compatible project using C++17 where required
- Official universal SDL 2.32.10 framework installed by the bootstrap script
- Reproducible GitHub Actions release build
- Application and embedded framework ad-hoc signed and verified during CI
- The Xcode project retains an `x86_64` architecture setting, but the current
  published release and automated validation target Apple Silicon

## 680x0 emulation

- Musashi-based MC68020 emulation
- Updated Musashi core documented in [MUSASHI.md](MUSASHI.md)
- Safe conversion between 32-bit emulated addresses and 64-bit host pointers
- Portable packed structures for the Atari ABI
- 68EC030/68030 and MMU modes are not enabled in the current release

## Display

- User-selectable Atari resolutions and colour depths
- Resizable SDL output window
- Native macOS fullscreen through the green window button or
  `Control-Command-F`
- HiDPI/Retina-aware drawable sizing
- Aspect-ratio-preserving scaling
- Zoom support for classic 640×400 and 640×200 modes
- Corrected framebuffer byte-lane access on ARM64
- Corrected 32-bit RGB/VDI colour conversion
- Fullscreen redraw and mouse-coordinate mapping corrections

## Mouse and keyboard

- Mouse input in windowed and fullscreen modes
- Full-screen pointer range follows the actual drawable area
- Coalesced mouse-motion handling for smooth movement with low emulator
  overhead
- Mouse wheel support for conventional scrolling mice
- MacBook trackpad scrolling support
- Wheel and trackpad scrolling are exposed to MagiC applications as
  Atari-compatible cursor-key events

## macOS integration

- Selected macOS folders can be mapped as Atari drives
- Reliable writable-drive operation on 64-bit hosts
- macOS-compatible file permission and ownership handling
- Compatibility handling for optional Finder Type/Creator metadata
- Drive synchronisation handling for file copies made by Jinnee
- Clipboard exchange between macOS and the emulated environment
- German, French and English localisation for AtariX and the emulated system
- Guest **Ausschalten** requests close AtariX cleanly

## Current compatibility boundaries

- A `MAGIC_C` folder must still be created and selected manually on first use.
- The downloadable application is ad-hoc signed, not notarised with an Apple
  Developer ID. Gatekeeper can therefore request manual confirmation once.
- AtariX still uses its historical MacXFS architecture. The HostXFS
  implementation in [MagicOnLinux](https://gitlab.com/AndreasK/magiclinux) is
  a useful reference for a possible later filesystem redesign.
- The emulator currently targets an MC68020 environment without an MMU. Tests
  requiring a true 68030 MMU should use another emulator or physical hardware.
- The cursor-key wheel translation is deliberately conservative for broad
  MagiC application compatibility; native application-specific wheel
  protocols can be evaluated later.

## Suggested future work

- Automate initial `MAGIC_C` creation and setup
- Re-evaluate MacXFS against current macOS filesystem APIs and HostXFS ideas
- Add automated guest-level regression tests for display, files and input
- Reproduce and assess the historical one-pixel width/height issue across
  supported screen drivers
- Evaluate optional 68EC030 compatibility without exposing incomplete MMU
  behaviour
