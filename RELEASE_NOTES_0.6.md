# ATARIX-A64-0.6-Release

Native Apple-Silicon-Ausgabe von AtariX für ARM64, gebaut unter macOS 15.

## Änderungen gegenüber 0.5

- Die Kanalzuordnung der 32-Bit-True-Color-Ausgabe wurde auf das von SDL und macOS erwartete RGB-Layout korrigiert. Bilder in PhotoLine und Calamus werden dadurch wieder mit den richtigen Farben dargestellt.
- Die Zeigerfortschaltung in den VDI-Funktionen `VsetRGB` und `VgetRGB` wurde korrigiert. Damit werden keine Paletteneinträge mehr übersprungen, überlappt oder außerhalb der Farbtabelle geschrieben.
- Ungültige beziehungsweise leere VDI-Farbanfragen werden vor dem Tabellenzugriff abgefangen.
- Das eingebettete SDL2-Framework und anschließend das gesamte App-Bundle werden beim GitHub-Build konsistent ad-hoc signiert. Dadurch meldet macOS das heruntergeladene Programm nicht mehr fälschlich als beschädigt.
- Die Versionsangabe im App-Bundle wurde auf 0.6 gesetzt und wird im Build automatisch geprüft.

## Technische Basis

- Apple Silicon / ARM64
- macOS Deployment Target 11.0
- Aktualisierter Musashi-680x0-Kern aus Version 0.5
- CPU-Modus weiterhin 68020, ohne MMU
- Universal gebautes SDL2-Framework

## Prüfung

Der 0.6-Stand wurde mit MagiC sowie unter anderem mit Jinnee und PhotoLine getestet. Fensterdarstellung, 32-Bit-Farben und Bildwiedergabe funktionieren im bestätigten Teststand korrekt.

## macOS-Hinweis

Die Anwendung ist ad-hoc signiert, aber nicht mit einem kostenpflichtigen Apple-Developer-ID-Zertifikat notarisiert. macOS kann deshalb beim ersten Start einmalig eine Bestätigung verlangen. In diesem Fall im Finder mit der rechten Maustaste auf `AtariX.app` klicken, **Öffnen** wählen und anschließend bestätigen.
