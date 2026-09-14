# AtariX 0.7.1: reversible host filenames

## Fixed

Copying a listed file such as `Font_∑ndern.PAP` or `Einf˝hrkurs.PAP`
failed with GEMDOS -33 because the UTF-8 fallback returned by listing was
interpreted as separate Atari characters during the subsequent lookup.

MacXFS now uses dedicated `HostFilenameToAtari` / `AtariFilenameToHost`
functions for names and paths. General text conversion remains unchanged.
Listing, Fxattr/stat, open/create, delete, rename, permissions, directory
paths, links and filesystem controls use the same filename codec. Errors
from conversion are checked: malformed encoding returns TOS_EINVAL;
insufficient capacity returns TOS_ERANGE. Failed conversions clear the
output instead of returning a usable truncated filename.

## Visible names and compatibility

- Ordinary ASCII and all exactly representable Atari characters keep their
  encoding. DOS names such as `LONGNA~1.TXT` are unchanged.
- Other Unicode scalars use `~{HHHH}` (4-6 hexadecimal digits):
  `Font_∑ndern.PAP` appears as `Font_~{2211}ndern.PAP`;
  `Einf˝hrkurs.PAP` appears as `Einf~{02DD}hrkurs.PAP`.
- These guest names decode back to the original host names on copying or
  creating a target. No original files are renamed and no umlauts guessed.
- A literal host `~{` is quoted as `~{007E}{`. Thus the literal filename
  `~{2211}` cannot collide with `∑`. Existing guest scripts spelling a
  literal `~{` must use the quoted spelling. Other tildes are unchanged.
- Valid UTF-8-looking Atari byte sequences are always interpreted as Atari
  bytes, never guessed as UTF-8. `Γêæ` remains different from `∑`.
- Filename normalization is deliberately disabled. NFC and NFD remain
  byte-reversible and distinct even on normalization-sensitive filesystems.
  Decomposed accents may therefore be visible as escapes (e.g. `a~{0308}`).
  The host filesystem's own case/normalization equivalence still applies.
- Standard 8.3 paths return only complete names that fit in 8.3. Expanded
  names that do not fit continue to require long-name APIs. No new short
  aliases or truncated escape sequences are fabricated. The old lookup
  fallback no longer interprets UTF-8 bytes as Atari bytes or accepts a
  truncated 8.3 alias that listing never returns.
- Existing path/name capacity limits still apply. Escape expansion may
  cause ERANGE on unusually long names; there is no silent truncation.
- This fixes character encoding; existing DOS path separator and wildcard
  semantics remain in effect. It is not a redesign for arbitrary POSIX
  filenames containing DOS-reserved path characters.
- `.DS_Store` filtering is unchanged.

## Build and regression tests (macOS / Apple Silicon)

Use Xcode 16 or newer and the pinned SDL 2.32.10 framework. From this folder:

```sh
bash scripts/bootstrap-sdl2.sh
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
python3 tests/run_safety_tests.py
xcodebuild -project src/AtariX-MT/AtariX/AtariX.xcodeproj \
  -scheme AtariX-Application -configuration Release -derivedDataPath build \
  ARCHS=arm64 ONLY_ACTIVE_ARCH=NO CODE_SIGNING_ALLOWED=NO build
```

The test runner compiles current production functions with AddressSanitizer.
Tests cover actual MacXFS listing -> attribute lookup -> open/read/copy,
create, rename, delete, directory traversal, Dgetpath, D(x)readdir,
Fsfirst/Fsnext, short names, links, literal escapes, Atari-byte ambiguity,
Unicode beyond the BMP, NFC/NFD, invalid/truncated input and buffer guards.
Existing memory/pixel/path/geometry regressions also run.

Optional read-only source-tree check:

```sh
python3 tests/run_safety_tests.py --copy-tree /path/to/Jinnee
```

The runner mounts that source read-only, copies into a new temporary folder
through production MacXFS calls and independently compares relative paths,
file sizes and SHA-256. It checks every original source file again afterward.
`.DS_Store` is excluded from the target comparison because AtariX hides it.
Source fixtures must contain only ordinary files and directories, no links.
No original target directory is touched.

## Runtime acceptance still required

These host tests call the production filesystem functions directly; they
are not ThinkST running in an emulated MagiC session. For acceptance, run
this 0.7.1 app with ThinkST 0.39g and an isolated copy of the Atari drive,
copy Jinnee into an empty destination, check TSFILE.LOG, then compare all
visible files and directories. Keep host-test and guest-runtime evidence
separate. Use the 0.7.1 application supplied alongside this source package.
