#!/usr/bin/sh
cd $(dirname $(dirname ${0})) || exit 1
cmake -B build/win64 -A x64 -DCMAKE_BUILD_TYPE=${1:-Debug} || exit 1
cmake --build build/win64 --config ${1:-Debug} --parallel || exit 1
exit 0
