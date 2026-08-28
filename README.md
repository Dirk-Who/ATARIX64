[![Build Status](https://travis-ci.org/th-otto/AtariX.svg?branch=master)](https://travis-ci.org/th-otto/AtariX)

Latest snapshot: [![Download](https://api.bintray.com/packages/th-otto/atarix-files/snapshots/images/download.svg) ](https://bintray.com/th-otto/atarix-files/snapshots/_latestVersion#files)

# Atari X
<img alt="Logo" src="src/AtariX-MT/AtariX/ApplicationIcon.png" width="80">

An Atari ST/TT emulator for macOS.

This is kind of successor of MagiC for Atari, MagicMac for Classic Mac OS and MagicMac X for macOS X.
Native Apple Silicon build instructions are in APPLE_SILICON.md.
Origin and Attribution
The vast majority of this codebase was written by Andreas Kromke (1990-2018), the original author of MagiC, MagicMac, MagicMac X and AtariX. His copyright notice is preserved in the individual source files throughout this repository.
Andreas Kromke's original source is at gitlab.com/AndreasK/AtariX.
This tree continues from Thorsten Otto's fork, which kept AtariX buildable on modern macOS toolchains after the original development ended.
This branch: Apple Silicon port
Modified by: Dirk Werth with CodeX
Date: 25.07.2026

Native Apple Silicon build instructions are in
[APPLE_SILICON.md](APPLE_SILICON.md).

***

Contrary to its predecessor it does not contain the highly optimized Asgard68k emulator, written in hand optimised PPC assembler, but a portable emulator called Musashi, written in plain C.

***

# Important

The original application was developed until the end of 2014. One still has to **manually create a directory MAGIC_C**, preferably in the Documents folder, then start AtariX, **select the newly created directory** as root drive (it appears as "C:" in emulation), initialise the drive, and start the emulator.

Historical releases were 32-bit applications. The current source tree targets
64-bit Intel and Apple Silicon Macs; see [APPLE_SILICON.md](APPLE_SILICON.md).

# Screenshots
<img alt="No" src="assets/Atari-Desktop.png" width="1024">
<img alt="Yes" src="assets/Settings-Common.png" width="640">
<img alt="Yes" src="assets/Settings-Drives.png" width="640">
<img alt="Yes" src="assets/Settings-Devices.png" width="640">


# Supported

* Emulates MC68020 processor
* Arbitrary screen sizes and colour depths
* Zoom, helpful for original 640x400 or 640x200 resolution
* Full access to macOS file system, up to root
* German, French and English localisation for emulator
* German, French and English localisation for emulated system

# Bugs and Agenda

* Musashi emulator sources must be synchronised with latest version (see below).
* MAGIC_C folder must be automatically created.
* Application should be created with latest XCode.
* tell me

# License

The AtariX emulator is licensed according to GPLv3, see LICENSE file.

# External Licenses

**Musashi 68k emulator in C**  
Copyright 1998-2002 Karl Stenerud  
Source: https://github.com/kstenerud/Musashi  
License: https://github.com/kstenerud/Musashi/blob/master/readme.txt  

**SDL library:**  
Source: https://www.libsdl.org  
Copyright: paultaylor@jthink.net  
License: https://www.libsdl.org/license.php  

**Atari VDI Drivers**  
Copyright: Wilfried und Sven Behne  
License: mit freundlicher Genehmigung  
