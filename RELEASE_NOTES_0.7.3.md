# AtariX 0.7.3 – Eingabepuffer-Korrektur

Basis: veröffentlichte Version 0.7.2, Commit
`a1094122d199ab3f19a6a892052cf064951f1e75` aus Dirk-Who/ATARIX64.
Aktuelle Version nach erfolgreichem praktischen Scrolltest durch den Nutzer.

## Korrekturen

- Mausrad-Schritte werden weiterhin als Atari-Pfeiltasten übergeben. Druck
  und Loslassen werden jetzt unter derselben Eingabesperre vollständig als
  Paar eingereiht. Sind weniger als zwei Plätze frei, wird der gesamte
  zusätzliche Schritt verworfen. Es verbleibt kein halber Mausrad-Tastendruck.
- Pro SDL-Mausradereignis werden höchstens acht Schritte verarbeitet. Sehr
  große positive/negative Werte werden vor einer Vorzeichenumkehr begrenzt,
  einschließlich INT_MIN. Natürliches Scrollen, horizontales Scrollen und
  der Vorrang der vertikalen Achse bei diagonalen Ereignissen bleiben erhalten.
- Bereits gepufferte Eingaben lösen auch bei 0, 1 oder 2 freien Plätzen ihren
  Tastatur-/Maus-Interrupt aus. Bisher wurde der Interrupt in diesem Zustand
  unterdrückt und zugleich seine Vormerkung gelöscht.
- Neue Mausposition und Maustastenzustand werden auch bei vollem Ringpuffer
  vorgemerkt. Diese Vormerkung schreibt noch keine Bytes in den Ring.
  Der Gast-Leseaufruf erzeugt erst nach dem Leeren der Tastenwarteschlange
  vollständige Drei-Byte-Mauspakete; dort wird freier Platz geprüft.
  Eine vorgemerkte Bewegung oder Loslassbewegung braucht deshalb kein neues
  Host-Ereignis, um nach dem Leeren zugestellt zu werden.

Die bisherige Zusammenfassung von Mausbewegungen auf den neuesten Zielpunkt
bleibt bestehen. Es wird keine unbegrenzte Warteschlange angelegt. Die
Behandlung einzelner physischer Tastendrücke und deren bisherige Rückgabe bei
Pufferüberlast wurden nicht neu entworfen. Dieser Fix garantiert vollständige
synthetische Mausrad-Paare und behebt die nachgewiesene IRQ-Unterdrückung.

Die 0.7.1-Dateinamenkorrektur und die 25/50-Hz-Bildausgabe aus 0.7.2 bleiben
enthalten. Umschaltung: **Ansicht → Bildausgabe → 25 Hz / 50 Hz**.

## Automatisierte Prüfung

`tests/run_input_tests.py` verwendet die Produktionsfunktionen für SDL-Mausrad,
Scancode-Umrechnung, Ringpuffer, Eingabe-IRQ-Vorbereitung und Gast-Leseaufruf,
die echte rekursive Eingabesperre sowie die unveränderte CMagiCMouse-Klasse.
Der 68k-Prozessor/IRQ-Eintritt wird kontrolliert ersetzt. ASan und UBSan sind aktiv.

Abgedeckt sind alle 32 Ringpositionen und alle 32 freien Kapazitäten, vier
Scrollrichtungen, Richtungswechsel, geflipptes/diagonales Scrollen, extreme
Deltas, fehlender Platz für ein Paar, Burstbegrenzung, Tastatur-/Mausmischlast,
verzögerte Mausbewegung und Maustastenfreigabe, byteweises Leeren ohne weitere
Hostereignisse, Bewegungen über mehrere Pakete und konkurrierende Produzenten.
Die Anzahl der akzeptierten Paare im Konkurrenztest hängt vom Scheduling ab;
daher variiert die Gesamtzahl der ausgeführten Eingabeprüfungen leicht.

Der zusätzliche Baseline-Modus hat beide Fehler am unveränderten 0.7.2-Code
reproduziert. Die ursprünglichen Speicher-, Dateinamen-, Grafik-, Pfad- und
Displaytests werden ebenfalls ausgeführt (82.293 Prüfungen).

## Build aus diesem Quellpaket

SDL2.framework 2.32.10 ist für den Offline-Neuaufbau enthalten. Unter macOS
mit `ditto` entpacken, damit die internen Framework-Verknüpfungen erhalten bleiben.
Voraussetzung: vollständig eingerichtetes Xcode mit bestätigter Lizenz.
Die veröffentlichte App ist exakt der vom Nutzer getestete Build, erstellt
mit Xcode 27 / SDK 27 für Apple Silicon ab macOS 12. Xcode 27 akzeptiert das bisherige Ziel macOS 11 nicht mehr.
Die Projektvorgabe bleibt für ältere geeignete Toolchains bei 11.0;
das folgende Kommando setzt ausdrücklich das Ziel des veröffentlichten Builds.

```sh
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
python3 tests/run_safety_tests.py
python3 tests/run_display_tests.py
python3 tests/run_input_tests.py
xcodebuild -project src/AtariX-MT/AtariX/AtariX.xcodeproj \
  -scheme AtariX-Application -configuration Release -derivedDataPath build \
  ARCHS=arm64 ONLY_ACTIVE_ARCH=NO MACOSX_DEPLOYMENT_TARGET=12.0 \
  CODE_SIGNING_ALLOWED=NO build
```

Optionaler Fehlernachweis mit einem separaten 0.7.2-Quellpaket:

```sh
python3 tests/run_input_tests.py --repo /path/to/ATARIX-0.7.2 --characterize-baseline
```

## Praktischer Scrolltest

Die Hosttests weisen die genannten Fehlerpfade und ihre Korrektur nach.
Nach dem Testbuild bestätigt der Nutzer: „Das hat jetzt geklappt“. Damit
ist die Korrektur auch im gemeldeten praktischen Scrolltest erfolgreich.
Das ist keine vollständige Kompatibilitätsprüfung aller MagiC-Gastprogramme
und Grafikmodi. Für weitere Vergleichstests die 0.7.2-App aufbewahren.

1. Einen großen Verzeichnisbaum öffnen, zügig in beide Richtungen scrollen
   und mehrmals unmittelbar die Richtung wechseln.
2. Gleichzeitig Maus bewegen, Ordner auswählen und normale Tasten benutzen.
   Nach dem Scrollen prüfen, dass Maus, Tastatur und Maustasten-Loslassen reagieren.
3. Den Vergleich bei 25 und 50 Hz sowie nach Vollbildwechsel wiederholen.
4. Version aus „Über AtariX“, Grafikmodus und beobachtetes Verhalten festhalten.

Kein ThinkST-Code und keine Gastdateien wurden durch diese Korrektur verändert.
Ein automatisierter vollständiger MagiC-/ThinkST-Gasttest liegt nicht vor.

## Release-Dateien und Prüfung

- `ATARIX-A64-0.7.3-Release.zip`: bytegleich mit dem vom Nutzer getesteten
  App-ZIP, mit unverändertem Inhalt. App und SDL sind ad-hoc signiert;
  keine Apple-Notarisierung. Voraussetzung: Apple Silicon und macOS 12+.
- `ATARIX-0.7.3-source.zip`: veröffentlichter Programmcode mit Release-Dokumentation
  und SDL2.framework 2.32.10 für Offline-Builds; keine vorgebaute AtariX.app.
- `SHA256SUMS.txt`: Prüfsummen beider ZIPs.

Lokal bestanden 82.293 bisherige Prüfungen und mehr als 210.000 neue
Eingabeprüfungen je Durchlauf, auch aus dem unabhängig entpackten Quell-ZIP.
Der ARM64-Neuaufbau und die Signatur-/Paketprüfung bestanden. Die
GitHub-Konfiguration führt alle drei Testsuiten sowie den ARM64-Build aus.
