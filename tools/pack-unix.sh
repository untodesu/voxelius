#!/usr/bin/sh
cd $(dirname $(dirname ${0})) || exit 1
${SHELL} tools/build-unix.sh Release || exit 1
cpack --config build/unix/CPackConfig.cmake -G ZIP || exit 1
exit 0
