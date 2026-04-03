#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

if [[ $# -lt 1 ]]; then
    echo "usage: $0 SWPFILE [OBJPREFIX]" >&2
    exit 1
fi

if [[ -S /mnt/wslg/runtime-dir/wayland-0 ]]; then
    export XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir
    export WAYLAND_DISPLAY=wayland-0
    export DISPLAY=:0
    export PULSE_SERVER=unix:/mnt/wslg/PulseServer
fi

if [[ -x /usr/bin/xdpyinfo ]] && ! xdpyinfo >/dev/null 2>&1; then
    echo "X display is not reachable from this WSL session." >&2
    echo "Try these in order:" >&2
    echo "1. On Windows PowerShell, run: wsl --shutdown" >&2
    echo "2. Reopen Ubuntu and rerun this script." >&2
    echo "3. If it still fails, verify WSLg is enabled or start a Windows X server." >&2
    exit 1
fi

exec "$BUILD_DIR/a1" "$@"
