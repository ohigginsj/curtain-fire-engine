#!/bin/sh

Standard="-std=c++11"
Warnings="-Wall -Wsign-compare -Wno-unused-variable -Wno-missing-declarations -Wno-gnu-anonymous-struct"
Options="-pedantic -shared -fPIC -g"
Includes="-I include -I include/freetype2"
Output="-o bin/cfe.so.new"
Libraries="-lz -lfreetype `sdl2-config --libs` -lSDL2_mixer"
Sources="src/game.cpp"

clang++-3.5 $Standard $Warnings $Includes $Options $Output $Sources $Libraries
mv -f bin/cfe.so.new bin/cfe.so
