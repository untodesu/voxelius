#!/bin/sh
${SHELL} build.unix.generic.sh || exit 1
cpack --config build/CPackConfig.cmake -G ZIP || exit 1
exit 0
