# AtariX 0.6.6 - Apple Silicon

AtariX 0.6.6 ergänzt die Weitergabe des macOS-Mausrads an MagiC und klassische GEM-Anwendungen.

## Änderung gegenüber 0.6.5

- Vertikale Mausradbewegungen werden als Atari-Cursor-hoch/-runter-Tastenklicks weitergegeben.
- Horizontale Mausradbewegungen werden als Cursor-links/-rechts-Tastenklicks weitergegeben.
- Die SDL-Markierung für natürliches Scrollen unter macOS wird berücksichtigt.
- Pro SDL-Ereignis werden höchstens acht Schritte übertragen, damit der kleine MagiC-Tastatur-/IKBD-Ringpuffer nicht überlastet wird.
- Die vorhandene Vollbild-Maussteuerung bleibt unverändert.

## Praktische Prüfung

- Die Scrollfunktion wurde mit einer Logitech-Scrollmaus praktisch geprüft und bestätigt.
- Das Scrollen über das Touchpad eines MacBook wurde ebenfalls praktisch geprüft und bestätigt.
- Die vorhandenen Vollbild-, Mausbewegungs-, Farb- und Ausschaltkorrekturen bleiben unverändert erhalten.

AtariX emuliert weiterhin einen MC68020 ohne MMU. Die App ist ad-hoc signiert und nicht mit einer Apple-Developer-ID notarisiert.

Die vollständige Änderungshistorie befindet sich in `docs/AtariX_Aenderungen_0.3_bis_0.6.6.pdf`.
