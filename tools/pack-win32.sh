#!/usr/bin/sh
cd $(dirname $(dirname ${0})) || exit 1
${SHELL} tools/build-win32.sh Release || exit 1
cpack --config build/win32/CPackConfig.cmake -G ZIP || exit 1
exit 0
