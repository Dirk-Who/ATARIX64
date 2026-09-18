# AtariX 0.7.4 – Maus bis zum Fensterrand

Im Fenstermodus konnte die Atari-Maus den unteren Bildrand nicht erreichen,
bevor der macOS-Zeiger wieder erschien. Ursache war eine doppelte Umrechnung:
SDL liefert absolute Mausereignisse bei aktivierter logischer Auflösung
bereits in Bildkoordinaten; AtariX wendete Skalierung und Bildversatz erneut an.

Version 0.7.4 entfernt diese zweite Umrechnung. Bei einem 660×495-Punkte-Fenster
mit 640×480-Atari-Auflösung wird dadurch die letzte Zeile 479 statt nur Zeile
464 erreicht. Auch der rechte Rand wird korrekt zugeordnet. Die optionale
Pixelverdopplung wird weiterhin berücksichtigt; die relative Vollbildsteuerung
bleibt unverändert.

## Prüfung

- 672 Prüfungen mit dem echten SDL-2.32.10-Ereignisfilter und dem aus dem
  Produktionscode extrahierten Maus-Handler: verschiedene Fenstergrößen,
  Seitenverhältnisse, vier Varianten der Pixelverdopplung, alle Ecken,
  Innenpositionen sowie relative Bewegung und Randbegrenzung.
- Derselbe Test weist im unveränderten 0.7.3-Quellpaket 66 fehlschlagende
  Prüfungen nach, darunter den fehlenden 15-Pixel-Streifen am unteren Rand.
- Bestehende Eingabe-, Display- und Sicherheitsregressionen werden ebenfalls
  ausgeführt. Maus-, Eingabe- und Displaytests verwenden ASan und UBSan.
- Der Benutzer bestätigt den praktischen Erfolg im Fenstermodus mit
  `0.7.3-mousefix1`: „okay, das funktioniert“.

Die Veröffentlichung wird mit den Versionskennungen 0.7.4 neu gebaut; die
getestete Mauskorrektur ist unverändert. Der praktische Test deckt nicht jede
Kombination aus Gastprogramm, Grafikmodus und Bildschirm ab.

## Downloads und Build

- `ATARIX-A64-0.7.4-Release.zip`: Anwendung für Apple Silicon, macOS 12 oder neuer.
- `ATARIX-0.7.4-source.zip`: Quellcode, Tests, Dokumentation und SDL2.framework
  2.32.10 für einen Offline-Neuaufbau. Mit `ditto` entpacken, damit die
  Framework-Verknüpfungen erhalten bleiben.
- `SHA256SUMS.txt`: Prüfsummen der beiden Archive.

Die App und das eingebettete SDL2-Framework sind ad-hoc signiert; die App ist
nicht Apple-notarisiert. Bestehende Einstellungen und Atari-Laufwerke können
weiterverwendet werden.

```sh
python3 tests/run_mouse_coordinates_tests.py
python3 tests/run_input_tests.py
python3 tests/run_display_tests.py
python3 tests/run_safety_tests.py
xcodebuild -project src/AtariX-MT/AtariX/AtariX.xcodeproj \
  -scheme AtariX-Application -configuration Release -derivedDataPath build \
  ARCHS=arm64 ONLY_ACTIVE_ARCH=NO MACOSX_DEPLOYMENT_TARGET=12.0 \
  CODE_SIGNING_ALLOWED=NO build
```

Ein Git-Checkout benötigt vorher `scripts/bootstrap-sdl2.sh`. Der optionale
Vergleichstest verwendet das ursprüngliche 0.7.3-Quellarchiv:

```sh
python3 tests/run_mouse_coordinates_tests.py --baseline-zip /path/to/ATARIX-0.7.3-source.zip
```
