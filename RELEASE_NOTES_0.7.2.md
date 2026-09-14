# AtariX 0.7.2 – Apple Silicon

Basis: 0.7.1 mit der bereits getesteten Korrektur für Sonderzeichen beim Kopieren.
GitHub-Basis: Dirk-Who/ATARIX64, f7a5df1ba4d9ed902f10f420d24ba4356543440a.
Aktuelle Version nach positiver Rückmeldung des Nutzers zum praktischen Betrieb
einschließlich der Umschaltung zwischen 25 und 50 Hz.

## Änderungen

- Die drei 8-/16-/32-Bit-Schreibfunktionen setzen das VRAM-Änderungssignal
  mit einem atomaren Release-Store. Das bisherige atomare Lesen-und-Setzen
  entfällt; die Bildausgabe übernimmt und löscht das Signal weiterhin atomar.
- Unter **Ansicht → Bildausgabe → 25 Hz / 50 Hz** ist die Host-Bildausgabe
  während des Betriebs umschaltbar. Standard: 50 Hz. Die Auswahl wird als
  `atariDisplayRefreshRate` gespeichert. Ungültige Werte ergeben 50 Hz.
  Das Menü ist auch auf Englisch und Französisch vorhanden.
- Maximal ein Bildaktualisierungsereignis wartet auf Verarbeitung. Bei einem
  SDL-Fehler oder einem herausgefilterten Ereignis ist ein neuer Versuch möglich.
  Während der Ausgabe gesetzte Änderungen bleiben für die nächste Ausgabe erhalten.
- Die Atari-Interrupts bleiben bei nominell 200 Hz und 50 Hz VBL.
  Unverändert bleiben CPU-Kern, Gastgeschwindigkeit und Farbkonvertierung.

50 Hz ermöglicht häufigere Bildaktualisierungen als die bisherigen 25 Hz;
dadurch können Scrollen und Mausbewegungen flüssiger erscheinen. Bei hoher
Auflösung erhöht sich zugleich die Arbeit für Konvertierung und Bildausgabe.
25 Hz bleibt deshalb auswählbar. Die Frequenzen sind Sollwerte des SDL-Timers,
keine garantierten Bildraten und kein Faktor zwei für die Emulationsleistung.

## Prüfung und Build

Das separate Release-Quellpaket enthält SDL2.framework 2.32.10 als
Build-Abhängigkeit und keine vorgebaute AtariX.app. Bei einem Git-Checkout
zuerst `bash scripts/bootstrap-sdl2.sh` ausführen. Release-Quellpaket unter
macOS mit `ditto` entpacken, damit Framework-Verknüpfungen erhalten bleiben.

```sh
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
python3 tests/run_safety_tests.py
python3 tests/run_display_tests.py
xcodebuild -project src/AtariX-MT/AtariX/AtariX.xcodeproj \
  -scheme AtariX-Application -configuration Release -derivedDataPath build \
  ARCHS=arm64 ONLY_ACTIVE_ARCH=NO CODE_SIGNING_ALLOWED=NO build
```

Die Sicherheitstests verwenden die Produktionsfunktionen mit AddressSanitizer.
Die Displaytests verwenden die Produktions-Schedulerklasse und die echte
SDL-Timerfunktion mit Test-Ereignisqueue, ASan und UBSan. Sie prüfen Bildraten,
Gasttakte, Stillstand, Warteschlangenbegrenzung, Fehlerwiederholung, Änderungen
während der Ausgabe, Frequenzwechsel, Zählerüberlauf und konkurrierende Zugriffe.
Die CI-Konfiguration führt beide Suiten und einen ARM64-Release-Build aus.
Lokal bestanden 74.435 Sicherheits-, 6.660 Dateinamen- und 1.198 Displayprüfungen
(insgesamt 82.293). Auch ein unabhängiger Neuaufbau aus dem Quell-ZIP bestand.

## Praktische Rückmeldung und weitere Abdeckung

Der Nutzer bestätigt: „Das funktioniert alles sehr gut“, einschließlich der
Umschaltung zwischen 25 und 50 Hz. Diese praktische Rückmeldung ersetzt keine
vollständige Kompatibilitätsmatrix aller Gastprogramme und Grafikmodi.
Für weitere Erprobung eine
Kopie des Atari-Laufwerks verwenden und 25/50 Hz beim Scrollen, Bewegen von
Fenstern, Vollbildwechsel und bei Kopieroperationen vergleichen. Insbesondere
nach Wechsel der Bildrate und nach Verdecken/Wiederherstellen auf ausbleibende
Bildaktualisierungen achten. Die automatisierten Hosttests bleiben von dieser praktischen Rückmeldung
getrennt; ein automatisierter kompletter MagiC-Gasttest liegt nicht vor.

## Release-Dateien

Das ARM64-App-ZIP enthält bytegenau den vom Nutzer erprobten Build.
App und SDL sind ad-hoc signiert, nicht Apple-notarisiert.
Das Quell-ZIP bildet denselben Programmcode mit aktualisierter Release-Dokumentation ab.
Die SHA-256-Prüfsummen stehen im Release-Asset `SHA256SUMS.txt`.
