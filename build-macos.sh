#!/bin/sh
rm -rf build/
cmake -S . -B build -G Xcode -DCMAKE_EXPORT_COMPILE_COMMANDS=1
