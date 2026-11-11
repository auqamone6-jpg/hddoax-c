#!/bin/bash
# Builds Ember for the iOS Simulator and opens it there.
#
#   ./run-in-simulator.sh
#
# A Mac with Xcode and Homebrew, Apple Silicon or Intel. Everything for the Simulator is kept
# apart from the iPhone build: prefix-sim/, .deps-build-sim/, build/inferno-sim.
#
# Experimental. The emulator was never built for the Simulator by its authors.
set -euo pipefail
cd "$(dirname "$0")"

[ "${1:-}" = "clean" ] && exit 0
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

# The Simulator runs code for the Mac's own processor: arm64 on Apple
# Silicon, x86_64 on an Intel (or AMD) Mac.
case "$(uname -m)" in
    arm64)  export IOS_ARCH=arm64 ;;
    x86_64) export IOS_ARCH=x86_64 ;;
    *) echo "Unknown processor: $(uname -m)" >&2; exit 1 ;;
esac
echo "==> Building for the $IOS_ARCH Simulator"
command -v brew >/dev/null || { echo "Homebrew is required: https://brew.sh" >&2; exit 1; }
xcrun --sdk iphonesimulator --show-sdk-path >/dev/null || { echo "Xcode with the iOS Simulator SDK is required." >&2; exit 1; }

EMULATOR_REF="${EMULATOR_REF:-ios}"
export IOS_SDK=iphonesimulator
export PREFIX="$PWD/prefix-sim"
export WORK="$PWD/.deps-build-sim"
export OUT="$PWD/cross-iossim.txt"

echo "==> Build tools"
brew install --quiet meson ninja pkg-config cmake autoconf automake libtool m4 qemu
export PATH="$(brew --prefix m4)/bin:$PATH"

if [ ! -d "$PREFIX/lib" ]; then
    echo "==> Dependencies for the Simulator (once)"
    scripts/build-ios-deps.sh
fi

if [ ! -d inferno-src ]; then
    echo "==> Emulator source"
    git clone --depth 1 -b "$EMULATOR_REF" https://github.com/MakrSas/Inferno.git inferno-src
fi

echo "==> Meson cross-file"
./make-cross-file.sh

if [ ! -f build/inferno-sim/build.ninja ]; then
    echo "==> Configure the emulator"
    meson setup build/inferno-sim inferno-src \
        --cross-file="$OUT" \
        -Dbuildtype=release -Dprefix="$PREFIX" \
        -Dshared_lib=true -Db_staticpic=true -Dwerror=false \
        -Dkvm=disabled -Dhvf=disabled -Dwhpx=disabled \
        -Dcocoa=disabled -Dgtk=disabled -Dsdl=disabled -Dcurses=disabled \
        -Dcoreaudio=disabled -Dcurl=disabled -Dlibssh=disabled -Dbzip2=disabled \
        -Dvnc=enabled -Dvnc_jpeg=disabled -Dvnc_sasl=disabled \
        -Dtools=disabled -Dcoroutine_backend=ucontext
fi

echo "==> Emulator library"
ninja -C build/inferno-sim libqemu-aarch64-softmmu.dylib

echo "==> App"
export INFERNO_KEYMAPS="${INFERNO_KEYMAPS:-$(brew --prefix)/share/qemu/keymaps}"
app/build-sim.sh

echo "==> Simulator"
open -a Simulator
if ! xcrun simctl list devices booted | grep -q Booted; then
    xcrun simctl boot "${SIM_DEVICE:-iPhone 17 Pro}"
fi
xcrun simctl bootstatus booted >/dev/null
BUNDLE_ID="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' app/.build-sim/Ember.app/Info.plist)"
xcrun simctl install booted app/.build-sim/Ember.app
xcrun simctl launch booted "$BUNDLE_ID"

echo
echo "Ember is running in the Simulator. Its folder for the disk is:"
echo "  $(xcrun simctl get_app_container booted "$BUNDLE_ID" data)/Documents"
