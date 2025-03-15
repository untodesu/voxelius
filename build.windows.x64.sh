#!/bin/sh
cmake -B build64 -A x64 || exit 1
cmake --build build64 --config Release --parallel || exit 1
exit 0
