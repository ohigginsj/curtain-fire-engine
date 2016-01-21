#!/bin/sh

mkdir -p bin

Options="-g -Wno-missing-declarations"
Includes="-I include -I include/freetype2"
Output="-o bin/linux-platform.exe"
Libraries="-ldl -lfreetype `sdl2-config --libs` -lSDL2_mixer"
Sources="src/main.cpp"

./build-lib.sh
clang++-3.5 $Options $Includes $Output $Sources $Libraries
