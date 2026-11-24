#!/bin/bash

set -e

echo "Building Desktop Clock..."

WINDRES="x86_64-w64-mingw32-windres"
GCC="x86_64-w64-mingw32-g++"

# Check if MinGW-w64 is installed
if ! command -v $GCC &> /dev/null; then
    echo "Error: MinGW-w64 not found!"
    echo "Install it with: sudo apt install mingw-w64"
    exit 1
fi

# Compile resources
echo "Compiling resources..."
$WINDRES resources/clock.rc -O coff -o resources/clock.res

# Compile and link
echo "Compiling application..."
$GCC -std=c++11 -Wall -Wextra -Wno-unused-parameter -DUNICODE -D_UNICODE \
     -mwindows -municode \
     src/main.cpp \
     src/clock_window.cpp \
     src/settings.cpp \
     src/renderer.cpp \
     src/tray_icon.cpp \
     src/world_format.cpp \
     src/world_clock.cpp \
     resources/clock.res \
     -o DesktopClock.exe \
     -lgdiplus -luser32 -lshell32 -lcomctl32 \
     -static-libgcc -static-libstdc++

echo "Build successful! Output: DesktopClock.exe"

# Made with Bob
