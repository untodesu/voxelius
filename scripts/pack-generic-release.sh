#!/bin/sh
cd "$(dirname $(dirname "${0}"))" || exit 1
${SHELL} scripts/build-generic-release.sh || exit 1
cpack -G ${1:-TGZ} --config build/generic-release/CPackConfig.cmake || exit 1
exit 0
