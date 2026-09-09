# AtariX 0.7 – Apple Silicon

Version 0.7 übernimmt die praktisch getestete Fehlerbereinigung auf Basis 0.6.6.

## Korrekturen

- Speichergrenzen bei Pixelkonvertierung, unvollständigen Pixelgruppen und
  16-/32-Bit-Zugriffen auf RAM und Videospeicher abgesichert.
- Ein-Pixel-Fehler bei Pixmap-Abmessungen und anfänglichen Textur-Upload korrigiert.
- Lange Dateipfade und UTF-8-Konvertierungen gegen Pufferüberschreibungen abgesichert.
- Symbolische Links auf Laufwerkswurzeln sowie `.`/`..`-Pfadbehandlung korrigiert.
- Fehler beim Erstellen eines Verzeichnisses werden korrekt zurückgemeldet.
- App-Versionsanzeige und NatFeats-Versionskennung auf 0.7 aktualisiert.

## Prüfung

- 74.339 automatisierte Host-Prüfungen mit AddressSanitizer bestanden.
- Native ARM64-Kompilierung mit Xcode; App und SDL2-Framework ad-hoc signiert.
- Der Benutzer bestätigt guten praktischen Betrieb einschließlich Vollbild;
  bislang wurden keine Auffälligkeiten gemeldet.
- Diese Rückmeldung ist keine vollständige Prüfung aller Programme, Farbtiefen,
  Bildschirm-/Offscreen-Treiber und laufwerksübergreifenden Links.

## Grenzen und Installation

Die unveränderten Bildschirmtreiber unterstützen nur 13 Bit für die Zeilenlänge.
Deshalb begrenzt der Host die Breite auf 2032 Pixel bei 32 Bit, 4080 Pixel bei
16 Bit und 4096 Pixel in den übrigen Modi. Planare Modi werden auf 16 Pixel
ausgerichtet. HostXFS und dessen Kernel-Anpassungen wurden nicht übernommen.

Für Apple Silicon, macOS-Deployment-Ziel 11 oder neuer. Die App enthält SDL2
2.32.10, ist ad-hoc signiert und nicht Apple-notarisiert. Bitte vorhandenes
`MAGIC_C` vor dem Wechsel sichern. AtariX emuliert weiterhin einen MC68020 ohne MMU.

Downloads: `ATARIX-A64-0.7-Release.zip` (Anwendung),
`ATARIX-0.7-source.zip` (Quellcode, Tests, Dokumentation) und `SHA256SUMS.txt`.
Details: [SAFETY_FIXES.md](SAFETY_FIXES.md).
