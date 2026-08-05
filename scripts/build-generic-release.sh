#!/bin/sh
cd "$(dirname $(dirname "${0}"))" || exit 1
cmake --preset generic-release || exit 1
cmake --build --preset generic-release || exit 1
exit 0
