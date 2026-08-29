# Musashi integration

AtariX 0.5 uses the Musashi 680x0 emulator from:

- Repository: https://github.com/kstenerud/Musashi
- Upstream branch: master
- Upstream commit: 313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd
- Upstream commit date: 2026-03-08

The generated opcode sources report Musashi generator version 4.60. Some
upstream source headers retain historical version labels; the pinned commit
above is the authoritative identifier for this import.

## AtariX-specific integration

AtariX continues to emulate an MC68020 and preserves the MagicMac host-call
opcodes 0x00c0 and 0x00c1. They are generated from m68k_in.c together with
the upstream opcode table.

For compatibility with the former AtariX core, M68K_USE_64_BIT remains
disabled. A dedicated cross-thread stop flag preserves instruction-boundary
handling of mouse, timer and VBL events.

Bus errors are raised synchronously while Musashi's active execution trap is
valid. Deferring them until after m68k_execute() returns would jump into an
expired stack frame on ARM64.

The 32-bit macOS framebuffer uses native little-endian pixel storage. AtariX
maps 68k byte accesses with address XOR 3 and word accesses with address XOR 2;
longword accesses remain unchanged. This preserves all colour channels during
MagiC raster operations and window movement.

## Regenerating opcode sources

From the AtariX source directory:

    cc -O2 -o m68kmake-host m68kmake.c
    ./m68kmake-host . m68k_in.c
    rm m68kmake-host
