#!/usr/bin/sh
cd $(dirname $(dirname ${0})) || exit 1
cmake -B build/unix/${1:-Debug} -DCMAKE_BUILD_TYPE=${1:-Debug} || exit 1
cmake --build build/unix/${1:-Debug} --config ${1:-Debug} --parallel $(nproc) || exit 1
exit 0
