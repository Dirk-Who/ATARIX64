# AtariX 0.7.2 – Release source and application

The current release is ATARIX-A64-0.7.2-Release. See RELEASE_NOTES_0.7.2.md
for changes, validation and the user-reported runtime confirmation.

- release/AtariX.app.zip contains the same user-tested ARM64 application as
  the separate release asset ATARIX-A64-0.7.2-Release.zip.
- The Git repository does not bundle SDL2.framework. Run
  scripts/bootstrap-sdl2.sh before building a Git checkout.
- The separate ATARIX-0.7.2-source.zip release asset includes SDL2.framework
  2.32.10 for offline builds, but no prebuilt AtariX application.
- That source asset has SOURCE_COMMIT.txt, SOURCE_SHA256SUMS.txt and
  SOURCE_SYMLINKS.json to identify its commit, files and framework links.
- Keep the application and source packages separate. Verify SHA256SUMS.txt
  before use. Use macOS ditto when extracting to preserve framework links.
