#!/bin/sh
${SHELL} build.windows.x32.sh || exit 1
cpack --config build32/CPackConfig.cmake -G ZIP || exit 1
exit 0
