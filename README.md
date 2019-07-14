# Modplug Mod-to-MP3 Convertor

# Overview
Converts "mod" files to WAV or MP3 using libmodplug library

# Installation
Requirements
- libmodplug from XMMS project installed (0.8.8.4)
- TagLib Installed (1.6.3)
- Lame MP3 Encoder installed (3.98.4)
- CMake 2.8.x installed

# Building
- Copy <source dir>/libmp3lame.pc to your system pkg-config directory (in the same tree as Lame MP3 is installed)
- Unpack the source to a directory
- Create a directory for the build output
cd <build directory>
cmake <path-to-source-directory>
make

The generated executable is src/modplug2mp3

# Future work
- Improve build system so binary goes to a more sensible location
- Re-instate windows support
- Upgrade package versions to latest
- Try to avoid usage of custom pkg-config file for Lame MP3
- Include error correction information (lame_set_error_protection)
- Improve conversion status reporting (seems slow to update during testing)

