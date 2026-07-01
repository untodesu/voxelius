#!/bin/sh
cd "$(dirname $(dirname "${0}"))" || exit 1
cmake -B build/generic-debug -DCMAKE_BUILD_TYPE=Debug || exit 1
cmake --build build/generic-debug --config Debug --parallel || exit 1
exit 0
