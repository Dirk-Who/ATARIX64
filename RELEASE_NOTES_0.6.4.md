# AtariX 0.6.4 - Apple Silicon

AtariX 0.6.4 behebt die Mausbegrenzung im macOS-Vollbildmodus und verbessert die Zeigerbewegung.

## Änderungen gegenüber 0.6

- Der Atari-Mauszeiger kann im Vollbild die gesamte emulierte Bildschirmfläche erreichen.
- Der native macOS-Vollbildmodus über den grünen Fensterknopf und Control-Command-F werden unterstützt.
- Im Vollbild verwendet AtariX relative SDL-Mausbewegungen mit eigener virtueller Atari-Position.
- Der Fenstermodus verwendet weiterhin absolute Fensterkoordinaten.
- Doppelte Maus-Skalierung wurde entfernt; Bruchteile werden mit Gleitkomma-Genauigkeit akkumuliert.
- Die macOS-Systembeschleunigung wird auch im relativen Vollbildmodus verwendet.
- Große oder maximierte Fenster werden nicht mehr durch eine grobe 90-Prozent-Schwelle als Vollbild eingestuft.
- Unveränderte oder synthetische Mauspositionen erzeugen keine unnötigen Musashi-Unterbrechungen oder MagiC-Mausinterrupts.
- SDL-Viewport und logische Koordinatentransformation werden nach relevanten Fenster- und Vollbildänderungen aktualisiert.

## Beibehaltene Korrekturen aus 0.6

- Korrekte RGB-Kanalzuordnung bei 32-Bit-True-Color.
- Korrigierte VDI-Farbtabellenzugriffe.
- Aktualisierter Musashi-Kern mit AtariX-/MagicMac-Hostcalls.
- ARM64-kompatible Speicher- und Busfehlerbehandlung.
- Beschreibbare gemappte macOS-Laufwerke.
- Ad-hoc signiertes App-Bundle mit eingebettetem SDL2-Framework.

## System und Build

- Native Anwendung für Apple Silicon (arm64).
- Erstellt mit Xcode auf macOS 15.
- Eingebettetes SDL 2.32.10 Framework.
- AtariX emuliert weiterhin einen MC68020 ohne MMU.
- Die App ist ad-hoc signiert, jedoch nicht mit einer Apple-Developer-ID notarisiert.

Falls macOS den ersten Start blockiert, kann die App im Finder mit Rechtsklick und **Öffnen** bestätigt werden.

Die vollständige Änderungshistorie befindet sich in `docs/AtariX_Aenderungen_0.3_bis_0.6.4.pdf`.
