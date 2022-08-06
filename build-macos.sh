#!/bin/sh
rm -rf build/CMakeFiles
rm -f  build/CMakeCache.txt
cmake -S . -B build -G Xcode -DCMAKE_EXPORT_COMPILE_COMMANDS=1
# cmake . --build ./build --target GEngine --config Release
