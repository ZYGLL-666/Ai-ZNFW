#!/usr/bin/env bash
# Build the ai_service native binary.
#
# Usage:
#   ./build.sh                 # host build (for local testing)
#   ./build.sh ndk <ndk-path>  # cross-compile for Android ABIs
#
# Output for ndk build lands in native/libs/<abi>/ai_service.
set -euo pipefail

cd "$(dirname "$0")"

MODE="${1:-host}"

if [ "$MODE" = "ndk" ]; then
    NDK="${2:-${ANDROID_NDK_HOME:-}}"
    if [ -z "$NDK" ]; then
        echo "error: NDK path required (build.sh ndk /path/to/ndk)" >&2
        exit 1
    fi
    "$NDK/ndk-build" -C . APP_BUILD_SCRIPT=Android.mk NDK_PROJECT_PATH=. NDK_APPLICATION_MK=Application.mk -B
    echo "Android binaries written to native/libs/<abi>/ai_service"
    exit 0
fi

# Host build for local verification.
mkdir -p build-host
g++ -std=c++17 -O2 -Wall -Wextra \
    src/main.cpp \
    src/common.cpp \
    src/logger.cpp \
    src/clean.cpp \
    src/suppress.cpp \
    src/service.cpp \
    src/action.cpp \
    src/boot.cpp \
    -Isrc \
    -o build-host/ai_service

echo "Host binary written to native/build-host/ai_service"
