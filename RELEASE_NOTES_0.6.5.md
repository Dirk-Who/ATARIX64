# AtariX 0.6.5 - Apple Silicon

AtariX 0.6.5 behebt das Einfrieren der Anwendung beim Ausschalten von MagiC.

## Änderung gegenüber 0.6.4

- Der MagiC-Hostcall `AtariExit()` beendet nicht mehr nur den 68k-Emulationsthread.
- Der Gast-Shutdown wird thread-sicher als SDL-User-Event an die Host-Ereignisschleife weitergegeben.
- Die SDL-Ereignispumpe wird kontrolliert verlassen.
- Anschließend wird AtariX regulär auf dem Cocoa-Hauptthread beendet.
- Die Desktop-Funktion **Ende** bleibt unverändert und betrifft weiterhin nur den Desktop.

## Praktische Prüfung

Das vollständige Beenden von AtariX über **Ausschalten** wurde unter MagiC praktisch geprüft und bestätigt.

## Beibehaltene Korrekturen aus 0.6.4 und 0.6

- Vollständig nutzbare und flüssigere Maussteuerung im macOS-Vollbildmodus.
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

Die vollständige Änderungshistorie befindet sich in `docs/AtariX_Aenderungen_0.3_bis_0.6.5.pdf`.
