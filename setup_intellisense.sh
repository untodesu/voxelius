#!/bin/sh
set -e
cd "$(dirname "$0")"
if command -v python3 >/dev/null 2>&1; then
    exec python3 scripts/setup_environment.py intellisense "$@"
else
    exec python scripts/setup_environment.py intellisense "$@"
fi
