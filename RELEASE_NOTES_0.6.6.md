# AtariX 0.6.6 - Apple Silicon

AtariX 0.6.6 ergänzt die Weitergabe des macOS-Mausrads an MagiC und klassische GEM-Anwendungen.

## Änderung gegenüber 0.6.5

- Vertikale Mausradbewegungen werden als Atari-Cursor-hoch/-runter-Tastenklicks weitergegeben.
- Horizontale Mausradbewegungen werden als Cursor-links/-rechts-Tastenklicks weitergegeben.
- Die SDL-Markierung für natürliches Scrollen unter macOS wird berücksichtigt.
- Pro SDL-Ereignis werden höchstens acht Schritte übertragen, damit der kleine MagiC-Tastatur-/IKBD-Ringpuffer nicht überlastet wird.
- Die vorhandene Vollbild-Maussteuerung bleibt unverändert.

## Zu prüfen

- Scrollen im aktiven Verzeichnisfenster von ThinkST.
- Scrollrichtung mit aktivierter und deaktivierter macOS-Einstellung „Natürliche Scrollrichtung“.
- Vertikales und, sofern von der Maus unterstützt, horizontales Scrollen.
- Keine festhängenden Pfeiltasten nach schnellem Scrollen.

AtariX emuliert weiterhin einen MC68020 ohne MMU. Die App ist ad-hoc signiert und nicht mit einer Apple-Developer-ID notarisiert.
