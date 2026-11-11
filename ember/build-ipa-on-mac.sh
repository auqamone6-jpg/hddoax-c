#!/bin/bash
# One command on a Mac: everything .github/workflows/build-ipa.yml does on a
# runner, in the same order, ending with app/Inferno.ipa.
#
#   ./build-ipa-on-mac.sh
#
# Needs Xcode and Homebrew. The first run builds the nine iOS dependencies
# (about twenty minutes); later runs reuse prefix/ and build/.
set -euo pipefail
cd "$(dirname "$0")"

# Xcode calls this with its action. There is nothing of Xcode's to clean.
[ "${1:-}" = "clean" ] && exit 0
# Xcode starts scripts without Homebrew on the path.
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

EMULATOR_REF="${EMULATOR_REF:-ios}"
export PREFIX="${PREFIX:-$PWD/prefix}"

command -v brew >/dev/null || { echo "Homebrew is required: https://brew.sh" >&2; exit 1; }
xcrun --sdk iphoneos --show-sdk-path >/dev/null || { echo "Xcode with the iOS SDK is required." >&2; exit 1; }

echo "==> Build tools"
brew install --quiet meson ninja pkg-config cmake autoconf automake libtool m4 qemu ldid
export PATH="$(brew --prefix m4)/bin:$PATH"

if [ ! -d "$PREFIX/lib" ]; then
    echo "==> iOS dependencies (once)"
    scripts/build-ios-deps.sh
fi

if [ ! -d inferno-src ]; then
    echo "==> Emulator source"
    git clone --depth 1 -b "$EMULATOR_REF" https://github.com/MakrSas/Inferno.git inferno-src
fi

# app/build.sh stops without ChefKiss's filesystem patcher beside the app.
# The address is the one RESTORE.md gives.
if [ ! -f tools/InfernoFSPatcher/src/main.cpp ]; then
    echo "==> Filesystem patcher source"
    mkdir -p tools
    git clone --depth 1 https://git.chefkiss.dev/AppleHax/InfernoFSPatcher tools/InfernoFSPatcher
fi

# The emulator's build names /opt/homebrew/include whenever the target is
# arm64, which an iPhone always is. On an Intel Mac Homebrew is in /usr/local
# and that folder is not there, so the build stops. An empty one is enough.
if [ ! -d /opt/homebrew/include ]; then
    if [ -n "${GITHUB_ACTIONS:-}" ]; then
        sudo mkdir -p /opt/homebrew/include /opt/homebrew/lib
    else
        echo "This Mac has no /opt/homebrew/include, which the emulator's build expects." >&2
        echo "Create it once with: sudo mkdir -p /opt/homebrew/include /opt/homebrew/lib" >&2
        exit 1
    fi
fi

echo "==> Meson cross-file"
./make-cross-file.sh

if [ ! -f build/inferno/build.ninja ]; then
    echo "==> Configure the emulator"
    meson setup build/inferno inferno-src \
        --cross-file=cross-ios-arm64.txt \
        -Dbuildtype=release -Dprefix="$PREFIX" \
        -Dshared_lib=true -Db_staticpic=true -Dwerror=false \
        -Dkvm=disabled -Dhvf=disabled -Dwhpx=disabled \
        -Dcocoa=disabled -Dgtk=disabled -Dsdl=disabled -Dcurses=disabled \
        -Dcoreaudio=disabled -Dcurl=disabled -Dlibssh=disabled -Dbzip2=disabled \
        -Dvnc=enabled -Dvnc_jpeg=disabled -Dvnc_sasl=disabled \
        -Dtools=disabled -Dcoroutine_backend=ucontext
fi

echo "==> Emulator library"
ninja -C build/inferno libqemu-aarch64-softmmu.dylib

echo "==> App"
export INFERNO_KEYMAPS="${INFERNO_KEYMAPS:-$(brew --prefix)/share/qemu/keymaps}"
(cd app && ./build.sh)

# app/build.sh writes beside itself on a GitHub runner, and here otherwise.
if [ -n "${GITHUB_ACTIONS:-}" ]; then
    echo "==> Done: $PWD/app/Inferno.ipa"
else
    echo "==> Done: $PWD/Inferno.ipa"
fi
