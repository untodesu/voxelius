#!/bin/sh
cd "$(dirname $(dirname ${0}))" || exit 1
cmake -B build/unix-release -DCMAKE_BUILD_TYPE=Release || exit 1
cmake --build build/unix-release --config Release --parallel || exit 1
exit 0
