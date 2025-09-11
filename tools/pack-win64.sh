#!/usr/bin/sh
cd $(dirname $(dirname ${0})) || exit 1
${SHELL} tools/build-win64.sh Release || exit 1
cpack --config build/win64/CPackConfig.cmake -G ZIP || exit 1
exit 0
