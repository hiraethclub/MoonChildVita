#!/usr/bin/env bash
# Usage: BuildGameVita.sh [Clean]
# Builds the PS Vita VPK. Runs inside the vitasdk/vitasdk Docker image automatically
# when VitaSDK isn't installed locally. The VPK ends up in Bin/Vita/.
#
# Environment overrides:
#   MOONCHILD_VITA_VERSION  Port version, defaults to the one in CMake/Platforms/Vita.cmake
#   VITASDK_IMAGE           Docker image to build in, defaults to the pinned one below
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

# Pinned so builds don't change under us when the image is updated
VITASDK_IMAGE="${VITASDK_IMAGE:-vitasdk/vitasdk@sha256:fd82d88ff456d118eef16533ebf9a3a6ae0e292684fb8a3a35f15f0707b4692d}"

CLEAN=0
for arg in "$@"; do
    case "$arg" in
        Clean) CLEAN=1 ;;
    esac
done

cd "$(dirname "${BASH_SOURCE[0]}")/.."
ROOT_DIR="$(pwd)"

if [[ ! -f Libraries/zlib/CMakeLists.txt || ! -f Libraries/pl_mpeg/pl_mpeg.h ]]; then
    echo "${R_ERR}Submodules are missing! Run: git submodule update --init --recursive${R_0}"
    exit 1
fi

# Work out the git hash out here, since git may not trust the repo inside a container
GIT_HASH="$(git -c safe.directory='*' rev-parse --short=7 HEAD 2>/dev/null || echo unknown)"
if [[ "$GIT_HASH" != "unknown" ]] && ! git -c safe.directory='*' diff --quiet HEAD -- 2>/dev/null; then
    GIT_HASH="${GIT_HASH}-dirty"
fi

# Not inside VitaSDK? Re-run this script inside the Docker image
if [[ -z "${VITASDK:-}" || ! -f "${VITASDK}/share/vita.toolchain.cmake" ]]; then
    if ! command -v docker >/dev/null 2>&1; then
        echo "${R_ERR}Neither VitaSDK nor Docker was found!${R_0}"
        exit 1
    fi
    echo "${R_LOG}Building inside ${VITASDK_IMAGE}...${R_0}"
    DOCKER_ARGS=(--rm -v "${ROOT_DIR}:/src" -w /src
                 -e MOONCHILD_GIT_HASH="${GIT_HASH}"
                 -e MOONCHILD_VITA_VERSION="${MOONCHILD_VITA_VERSION:-}"
                 -e NO_COLOR="${NO_COLOR:-}")
    if [[ "$(id -u)" != "0" ]]; then
        DOCKER_ARGS+=(-u "$(id -u):$(id -g)" -e HOME=/tmp)
    fi
    exec docker run "${DOCKER_ARGS[@]}" "${VITASDK_IMAGE}" bash Scripts/BuildGameVita.sh "$@"
fi

GIT_HASH="${MOONCHILD_GIT_HASH:-$GIT_HASH}"
BUILD_DIR="CMakeOut/Vita"

if [[ "$CLEAN" == "1" ]]; then
    echo "${R_LOG}Cleaning ${BUILD_DIR}...${R_0}"
    rm -rf "${BUILD_DIR}"
fi

# Always passed, so an override from an earlier run doesn't stick in the cache
CONFIGURE_ARGS=(--preset vita -DMOONCHILD_GIT_HASH="${GIT_HASH}" -DMOONCHILD_VITA_VERSION="${MOONCHILD_VITA_VERSION:-}")

echo "${R_LOG}Configuring the game... (Vita / Release)${R_0}"
cmake "${CONFIGURE_ARGS[@]}"

echo "${R_LOG}Building the game...${R_0}"
cmake --build --preset build-vita-release --target MoonChildFE.vpk-vpk --parallel "$(nproc)"

# shellcheck disable=SC1091
source <(grep -E '^(VERSION|HASH|TITLE_ID)=' "${BUILD_DIR}/VitaBuildInfo.txt")
OUT_DIR="Bin/Vita"
OUT_NAME="MoonChildFE-vita-${VERSION}-${HASH}"
mkdir -p "${OUT_DIR}"
cp "${BUILD_DIR}/MoonChildFE.vpk" "${OUT_DIR}/${OUT_NAME}.vpk"
cp "${BUILD_DIR}/MoonChildFE" "${OUT_DIR}/${OUT_NAME}.elf"

echo "${R_OK}Build complete! ${OUT_DIR}/${OUT_NAME}.vpk (title ID ${TITLE_ID})${R_0}"
