#!/bin/sh
cd "$(dirname $(dirname ${0}))" || exit 1
${SHELL} scripts/build-unix-release.sh || exit 1
cpack -G ${1:-TGZ} --config build/unix-release/CPackConfig.cmake || exit 1
exit 0
