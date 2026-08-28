# Consolidated Apple Silicon source package

This package contains the source used for the Apple Silicon AtariX build.
The application version is 0.4F.

Important:

- No prebuilt `AtariX.app` is included. Older binaries do not contain all
  source fixes.
- No bundled `SDL2.framework` is included. Run `scripts/bootstrap-sdl2.sh`
  once before building. The script preserves the framework symlinks on
  macOS 26 and avoids the earlier `ditto: ... Is a directory` failure.
- The obsolete forced `M_DRV_READONLY` assignments have been removed from
  `MacXFS.cpp`, so mapped host directories are writable when macOS permits it.
- `XFSDevFunctions()` rebuilds the native drive pointer from the emulated
  file descriptor before write operations. This fixes the Apple Silicon
  `EXC_BAD_ACCESS` at address `0x8` seen when starting with writable drives.
- Version 0.4D keeps the association between each native host file descriptor
  and its Atari drive in host-side state. This fixes the 0.4C regression where
  folders could be created but copied files were silently discarded because
  the write call could not reconstruct its drive.
- Version 0.4E returns success after a successful `Fchmod` operation and
  provides macOS-compatible `Fchown` behaviour. Jinnee therefore no longer
  treats those two metadata calls as copy failures.
- Version 0.4F supplies compatible empty Finder Type/Creator metadata on modern
  macOS file systems and acknowledges mapped-drive sync requests. Jinnee can
  therefore complete a copy instead of deleting the successfully written
  destination after the obsolete metadata call returned error `-25`.
- The version information reads the bundle version without producing a
  spurious `(null)` line and credits the ARM64/SDL update on two lines
  immediately above the compilation date.
- Fullscreen mode is available from the View menu, with Control-Command-F,
  Alt-Return, or the macOS window controls. The SDL renderer preserves the
  Atari aspect ratio and maps mouse coordinates back to the emulated screen.
- In fullscreen mode the macOS host pointer is hidden, leaving only the Atari
  pointer visible. The host pointer is restored when AtariX loses focus,
  leaves fullscreen, or closes. Version 0.4C also recognises the native macOS
  fullscreen Space created by the green window button and restores the
  previously ignored "Hide host mouse cursor" preference.
- `EmulationRunner.cpp` uses public SDL keyboard event APIs instead of the
  removed private `SDL_KeyboardActivate()` function.

Follow `APPLE_SILICON.md` to create a fresh Release build. Test the application
directly from `build/Build/Products/Release/AtariX.app` before copying it to
`/Applications`.
