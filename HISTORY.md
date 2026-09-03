# AtariX development history

This is the concise public history of the modern macOS continuation. The
detailed German record is available in
[AtariX_Aenderungen_0.3_bis_0.6.6.pdf](docs/AtariX_Aenderungen_0.3_bis_0.6.6.pdf).

## Project lineage

- **Original AtariX:** developed by Andreas Kromke as a successor to MagiC,
  MagicMac and MagicMac X.
- **Toolchain continuation:** Thorsten Otto maintained a public fork and added
  fixes and improvements for newer development environments.
- **Modern macOS continuation:** Dirk Werth maintains this independent branch
  for current Apple Silicon Macs, with development assistance from OpenAI
  Codex.

The modern continuation retains the GPLv3 licence and the existing copyright
notices. It is not an official release by the original author.

## Version history

### 0.3 — historical baseline

- Last established Intel-era baseline used for this continuation
- Original AtariX/MagiC integration and MacXFS host-drive support
- Foundation for the subsequent 64-bit and Apple Silicon work

### 0.4 — Apple Silicon port

- Converted remaining 32-bit and Intel host assumptions for native `arm64`
- Replaced obsolete Carbon Multiprocessing Services with C++17 threads,
  condition variables and recursive mutexes
- Replaced the obsolete Carbon `Point` dependency with a private 16-bit type
- Converted 68k callback addresses through emulated RAM instead of truncating
  64-bit host pointers
- Added portable packing for Atari ABI structures
- Updated the Xcode project and SDL2 integration for modern macOS
- Added resizable and fullscreen output with corrected scaling and mouse
  coordinates
- Restored reliable writable mapped drives through native descriptor-to-drive
  state outside the emulated address space

### 0.4B — visible version identification

- Corrected application and build metadata so test builds could be identified
  reliably

### 0.4C — native fullscreen integration

- Improved recognition of native macOS fullscreen transitions
- Corrected cursor and window state during mode changes

### 0.4D — writable drive correction

- Reworked file-descriptor-to-drive mapping for 64-bit hosts
- Fixed write operations on mapped macOS folders

### 0.4E — macOS file semantics

- Adjusted `chmod` and `chown` handling to macOS-compatible success semantics
- Prevented compatible host filesystem behaviour from being reported as a
  guest error

### 0.4F — Finder metadata and copy synchronisation

- Made legacy Finder Type/Creator metadata optional on current macOS
- Corrected drive synchronisation handling so files copied by Jinnee remain
  present

### 0.5 — Musashi and ARM64 correctness

- Updated the integrated Musashi core to upstream commit
  `313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd`
- Retained AtariX-specific integration and callbacks
- Corrected ARM64 bus-error behaviour
- Corrected framebuffer byte-lane access that could corrupt colours or crash
  the emulator

### 0.6 — colour and release packaging

- Corrected RGB channel and VDI colour conversion for 32-bit display modes
- Fixed colour changes and contaminated window contents during redraw and
  movement
- Added reproducible Apple Silicon packaging
- Added ad-hoc signing of the application and bundled SDL2 framework, reducing
  misleading “damaged application” failures caused by inconsistent bundles

### 0.6.4 — fullscreen mouse and performance

- Updated the mouse coordinate range after native fullscreen transitions
- Removed the former restriction to the window's pre-fullscreen area
- Coalesced mouse-motion processing for smoother movement
- Recovered the emulator performance lost in early fullscreen-mouse test
  builds, in both windowed and fullscreen operation

### 0.6.5 — clean guest shutdown

- Connected MagiC's **Ausschalten** operation to a clean AtariX application
  shutdown
- Prevented the previous frozen-emulation state after guest power-off

### 0.6.6 — mouse wheel and trackpad

- Added scrolling for conventional mouse wheels
- Added scrolling from MacBook trackpads
- Translated scroll input into compatible Atari cursor-key events
- Confirmed operation with a Logitech scrolling mouse and a MacBook trackpad

## Release policy

Release builds are compiled by GitHub Actions for Apple Silicon, include the
required SDL2 framework, and are ad-hoc signed and verified. They are not
Apple-notarised. Every functional change should first be tested under MagiC
and then recorded here and in the detailed change log before publishing a new
release.
