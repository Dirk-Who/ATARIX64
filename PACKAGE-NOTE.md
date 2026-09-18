# AtariX 0.7.4 – Release source and application

Current release: ATARIX-A64-0.7.4-Release. See RELEASE_NOTES_0.7.4.md
for the windowed mouse-coordinate fix, validation and the successful user test.

- release/AtariX.app.zip contains the same ARM64 application as
  the release asset ATARIX-A64-0.7.4-Release.zip, requiring macOS 12 or newer.
  The mouse correction was user-tested in
  0.7.3-mousefix1; the release is rebuilt with version identifiers 0.7.4.
- The Git repository does not bundle SDL2.framework. Run
  scripts/bootstrap-sdl2.sh before building a Git checkout.
- The separate ATARIX-0.7.4-source.zip release asset includes SDL2.framework
  2.32.10 for offline builds, but no prebuilt AtariX application.
- That source asset includes SOURCE_COMMIT.txt, SOURCE_SHA256SUMS.txt and
  SOURCE_SYMLINKS.json to identify its commit, files and framework links.
- Application and source packages are separate. Verify SHA256SUMS.txt and
  use macOS ditto to preserve framework links when extracting.
- Release/CI builds set MACOSX_DEPLOYMENT_TARGET=12.0. The project retains
  its 11.0 default for older compatible toolchains; Xcode 27 needs 12.0+.
