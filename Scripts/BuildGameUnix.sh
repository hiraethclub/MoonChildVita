#!/usr/bin/env bash
# Usage: BuildGameUnix.sh [Debug|Release] [LinuxX64|LinuxArm64]
set -euo pipefail

if [[ -t 1 && -z "${NO_COLOR:-}" ]]; then
    R_ERR=$'\033[41m'
    R_OK=$'\033[32m'
    R_LOG=$'\033[35m'
    R_0=$'\033[0m'
else
    R_ERR=''
    R_OK=''
    R_LOG=''
    R_0=''
fi

CONFIG=Debug
ARCH_EXPORT=LinuxX64
PROC_COMMAND="nproc"

[[ $OSTYPE == 'darwin'* ]] && ARCH_EXPORT=macOSUniversal
[[ $OSTYPE == 'darwin'* ]] && PROC_COMMAND="sysctl -n hw.logicalcpu"

for arg in "$@"; do
    case "$arg" in
        Release) CONFIG=Release ;;
        Debug)   CONFIG=Debug ;;
        LinuxX64|LinuxArm64|macOSUniversal) ARCH_EXPORT="$arg" ;;
    esac
done

cd "$(dirname "${BASH_SOURCE[0]}")/.."

PRESET_BASE="$(printf '%s' "$ARCH_EXPORT" | tr '[:upper:]' '[:lower:]')"
PRESET="${PRESET_BASE}"
BUILD_PRESET="build-${PRESET_BASE}-debug"
if [[ "$CONFIG" == "Release" ]]; then
    BUILD_PRESET="build-${PRESET_BASE}-release"
fi

echo "${R_LOG}Configuring the game... ($ARCH_EXPORT / $CONFIG)${R_0}"
cmake --preset "$PRESET"

echo "${R_LOG}Building the game...${R_0}"
cmake --build --preset "$BUILD_PRESET" --target "MoonChildFE" --parallel "$(eval $PROC_COMMAND)"

echo "${R_OK}Build complete! ($ARCH_EXPORT / $CONFIG).${R_0}"
