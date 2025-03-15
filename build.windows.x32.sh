#!/bin/sh
cmake -B build32 -A Win32 || exit 1
cmake --build build32 --config Release --parallel || exit 1
exit 0
