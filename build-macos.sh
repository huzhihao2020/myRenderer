#!/bin/sh
rm -rf build/CMakeCache.txt
cmake -S . -B build -G Xcode
