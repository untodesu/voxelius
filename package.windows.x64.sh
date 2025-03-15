#!/bin/sh
${SHELL} build.windows.x64.sh || exit 1
cpack --config build64/CPackConfig.cmake -G ZIP || exit 1
exit 0
