# Version 0.7.4 validation (2026-09-18)

- Windowed coordinate regression: 672 checks passed using real SDL 2.32.10
  event filtering and the extracted production handler under ASan/UBSan.
- Original 0.7.3 baseline: 66 failing assertions reproduce the regression;
  a 660x495 window ends at guest y=464 instead of y=479.
- Input suite: 210435 assertions passed under ASan/UBSan (concurrency count varies).
- Display suite: 1198 assertions passed under ASan/UBSan.
- Safety suite: 74435 assertions passed.
- ARM64 Release build succeeded with Xcode 27, deployment target macOS 12.0.
  App version and NatFeats version string are 0.7.4.
- The user confirmed successful windowed operation of the same mouse fix in
  0.7.3-mousefix1. The 0.7.4 rebuild changes the version identifiers.
- App and embedded framework are ad-hoc signed. Existing build warnings remain.

Earlier release validation follows for historical reference.

# Validierung für AtariX 0.7

Basiscommit: `2f32cea89312d939d35cc1b522c98581da255334` plus die Sicherheits-
korrekturen und Versionsanpassungen für 0.7. Frischer Release-Build mit
Xcode 26.6 / Apple Clang 21 und SDL 2.32.10.

## Regressionstests

Aufruf: `python3 tests/run_safety_tests.py`

```text
PASS memory: endian modes, unaligned access, RAM/VRAM crossing, invalid spans
PASS pixels: six formats, aligned/partial groups, exact source strides, guard pixels
PASS XFS: long paths, root/relative/nested links, loops, missing paths, dot/parent handling
PASS UTF-8: complete sequences, NUL termination, truncation status
PASS geometry: all colour modes stay within bundled-driver pitch limits
PASS: 74339 assertions
```

Exitcode 0, keine AddressSanitizer-Meldung. Die Ausgabe
`cannot convert $2603 to atari codeset` gehört zum absichtlichen Test eines
nicht darstellbaren Unicode-Zeichens. Leak-Erkennung ist ausgeschaltet.

## Gesamtbuild

```sh
env DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer \
xcodebuild -quiet \
  -project src/AtariX-MT/AtariX/AtariX.xcodeproj \
  -scheme AtariX-Application -configuration Release \
  -derivedDataPath build \
  ARCHS=arm64 ONLY_ACTIVE_ARCH=NO CODE_SIGNING_ALLOWED=NO build
```

Exitcode 0. `file` bestätigt `Mach-O 64-bit executable arm64` für
`AtariX.app/Contents/MacOS/AtariX`. SDL-Runtime: 2.32.10 aus der lokal installierten
App; fehlende Headers aus dem offiziellen SDL-Tag `release-2.32.10`, Commit
`5d249570393f7a37e037abf22cd6012a4cc56a71`. Es wurde kein SDL-Code geändert.

Der Build ist nicht warnungsfrei: u. a. bestehende Zeitstempel-Konvertierungen,
`sprintf`, `memset` eines C++-Objekts und eine Build-Phase ohne Ausgabedeklaration.
Xcode meldet zusätzlich nicht verfügbare Simulator-Dienste; der macOS-Build
wird trotzdem erfolgreich abgeschlossen.

Die Bundle-Version wurde als `0.7` und die ausführbare Datei als ausschließlich
`arm64` verifiziert. App und Framework sind ad-hoc signiert; die Prüfung mit
`codesign --verify --deep --strict` ist auch nach unabhängiger ZIP-Extraktion
erfolgreich. Alle entpackten Dateien und Framework-Symlinks stimmen mit dem
signierten Original überein. Das Repository-Archiv `release/AtariX.app.zip`
enthält ebenfalls diesen neuen 0.7-Build.

SHA-256 der App-ZIP-Datei:
`21731862f292257b2f568b3324733b856b999bec8c5442da9145542f2bff1085`.

Der Benutzer meldet guten praktischen Betrieb des funktional gleichen
Korrekturstands einschließlich Vollbild, ohne bisher aufgefallene Fehler.
Der neu mit Versionskennung 0.7 gebaute Stand wurde nicht erneut im Gast
gestartet. Es liegt keine vollständige Matrix aller Programme und Farbtiefen vor.
Persönliche Laufwerke und installierte Anwendungen wurden nicht geändert.
Die App ist nicht Apple-notarisiert.

Weitere Laufzeitprüfungen: [SAFETY_FIXES.md](../SAFETY_FIXES.md).
