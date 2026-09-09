# Fehlerbereinigung in AtariX 0.7

Basis: `2f32cea89312d939d35cc1b522c98581da255334`.
Entstanden auf dem Arbeitszweig `fix/0.6.6-memory-display-paths`.
Die Korrekturen werden als Version 0.7 veröffentlicht.
MagiC-Kernel und mitgelieferte Bildschirmtreiber bleiben unverändert.

## Änderungen

- `EmulationRunner.cpp`: Die Pixelkonvertierung schreibt bei ungeraden Breiten
  nur die tatsächlich vorhandenen Pixel. Quell- und Zielzeilen werden auf
  ausreichende Länge geprüft; planare Daten benötigen vollständige 16-Pixel-Gruppen.
- `EmulationRunner.cpp`, `MagiC.cpp`: Pixmap-Rechts-/Untergrenze sind exklusiv
  (`w`, `h` statt `w-1`, `h-1`); die Videospeicherberechnung folgt derselben
  Konvention. Der zusätzliche Pixelverlust durch die Treiber entfällt im Host-Code.
- `EmulationRunner.cpp`: Unzulässige Zeilenlängen werden bereits bei der
  Konfiguration vermieden. Das anfängliche Testrechteck wird als ganze Oberfläche
  hochgeladen, damit bei 200 Zeilen kein ungeclipptes Rechteck gelesen wird.
- `MagiC.cpp`: 16-/32-Bit-Zugriffe prüfen den gesamten Adressbereich vor dem
  Zugriff. Nicht ausgerichtete Zugriffe verwenden die vorhandenen Bytefunktionen,
  auch beim Übergang zwischen getrenntem RAM und endian-konvertiertem Videospeicher.
- `MacXFS.cpp/.h`: Pfadaufbau ist kapazitätsbegrenzt; Aufrufer behandeln Fehler.
  Lange Pfade werden abgelehnt statt Puffer zu überschreiben. `.` und `..` werden
  komponentenweise verarbeitet; fehlende Verzeichnisse liefern einen Pfadfehler.
- `MacXFS.cpp`: Absolute symbolische Links auf ein eingebundenes Laufwerkswurzel-
  verzeichnis werden korrekt übersetzt. Wurzeln werden kanonisiert (auch macOS-
  Aliase wie `/var` und `/private/var`), Verzeichnisgrenzen respektiert und bei
  verschachtelten Einbindungen die längste passende Wurzel gewählt. Relative
  Linkziele bleiben relativ; POSIX behandelt Verzeichnislinks und Linkschleifen.
- `TextConversion.cpp/.h`: Beide Konvertierungsfunktionen melden Abschneiden
  über einen booleschen Rückgabewert. Es bleiben Platz für das Nullbyte und
  vollständige UTF-8-Sequenzen. XFS lehnt zu lange Eingabepfade und Linkziele ab.
- `MacXFS.cpp`: Ein fehlgeschlagenes `mkdir` liefert jetzt den Fehler zurück,
  statt dem Atari-System Erfolg zu melden.

## Bewusste Auflösungsgrenzen

Die mitgelieferten MFM-Treiber maskieren `rowBytes` mit `0x1fff`, also 13 Bit.
Eine Zeile muss deshalb kürzer als 8192 Byte bleiben. Bis zu einer separat
getesteten Treiberänderung begrenzt der Host die konfigurierte Breite:

| Modus | Maximale Breite |
| --- | ---: |
| 32 Bit / 16 Millionen Farben | 2032 Pixel |
| 16 Bit / High Color | 4080 Pixel |
| Übrige Modi | 4096 Pixel |

Die Grenzen enthalten Reserve für 16-Pixel-Ausrichtung. Planare 4-/16-Farben-Modi
werden auf ein Vielfaches von 16 Pixeln aufgerundet. Mindestbreite: 320 Pixel;
Höhenbereich unverändert: 200–2048 Pixel. Die tatsächlich verwendete Breite wird
protokolliert; die gespeicherte Benutzereinstellung wird dabei nicht umgeschrieben.
Dies ist eine Schutzbegrenzung, **keine Freigabe für 4K-True-Color**.

## Automatische Prüfung

Unter macOS mit Python 3, clang++ und SDL2-Headers:

```sh
./scripts/bootstrap-sdl2.sh
python3 tests/run_safety_tests.py
```

Der Testgenerator extrahiert die aktuellen Produktionsfunktionen und kompiliert
sie mit AddressSanitizer. Es gibt keine zweite, nachprogrammierte Implementierung
der zu testenden Funktionen. Der Speicherbus verwendet Testpuffer und einen
Busfehlerzähler; XFS verwendet seine echte Klasse und Zeigerzuordnung. Die
Textkonvertierung wird einschließlich CoreFoundation eingebunden. Die SDL-
Oberflächen sind Testdaten; es wird kein GUI/Renderer gestartet.

Geprüft werden gültige/ungültige Byte-, Wort- und Langwortzugriffe, beide Video-
Endian-Modi, RAM/VRAM-Übergänge, sechs Pixelkonvertierungen, teilweise Pixelgruppen,
Zeilenlängen und Schutzpixel, lange Pfade, relative/absolute/verschachtelte Links,
Linkschleifen, fehlende Pfade, `.`/`..`, Verzeichniserstellung einschließlich
Fehler- und Schreibschutzfall sowie Konvertierung bei kleinen Zielpuffern.

Testdaten entstehen in einem neuen temporären Verzeichnis. Das persönliche
`MAGIC_C` wird nicht verwendet. Leak-Erkennung ist deaktiviert, da die bestehende
Lebensdauer des XFS-Verzeichnisbaums nicht Gegenstand dieses Patches ist.

## Praktische Rückmeldung und weitere Laufzeitprüfungen

Ein vollständiger ARM64-Release-Build wurde mit Xcode und SDL 2.32.10 geprüft.
Der Benutzer hat den Korrekturstand praktisch getestet und guten Betrieb auch
im Vollbild ohne bisher aufgefallene Fehler bestätigt. Die genaue Auflösungs-,
Farbtiefen- und Programmmatrix wurde dabei nicht protokolliert. Das ist keine
vollständige Kompatibilitätsfreigabe für alle Atari-Programme.
Für weitergehende Prüfungen mit einer **Kopie** des eigenen `MAGIC_C` testen:

1. MagiC/Jinnee starten; 640×400 und 1024×768 sowie alle Farbtiefen prüfen.
2. Letzte Pixelzeile/-spalte, Maus an allen Rändern, Fensterwechsel, Zoom und
   Vollbild prüfen; zusätzlich 320×200 und Breite 321 ausprobieren.
3. Hohe Breiten an den obigen Grenzen prüfen; Desktop-Geometrie und Zeilen-
   versatz kontrollieren. Treiber, die eigenen/offscreen Speicher verwalten,
   sind gesondert zu testen.
4. Dateien und Verzeichnisse anlegen, kopieren, umbenennen und löschen;
   Groß-/Kleinschreibung, 8.3-Namen, Umlaute und tiefe Verzeichnisse prüfen.
5. Relative und absolute symbolische Links einschließlich Laufwerkswurzel,
   verschachtelter Einbindungen, defekter Links und Schleifen ausprobieren.
6. Gast ordentlich ausschalten und erneut starten.

MagicOnLinux/HostXFS wurde **nicht** übernommen. Insbesondere die Kernel-
Schnittstelle für laufwerks-/XFS-übergreifende Links ist nicht mit diesen
Korrekturen gelöst. Nicht darstellbare Unicode-Zeichen behalten das historische
Fallback-Verhalten; eine vollständige Unicode-Neukonzeption ist nicht enthalten.
Bestehende Compilerwarnungen wurden nicht pauschal unterdrückt oder bereinigt.
