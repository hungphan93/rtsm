#!/bin/bash

set -ex  # Exit on error, and print all commands

# === CONFIGURATION ===
REPO_ROOT=$(readlink -f "$(dirname "$0")")
BUILD_DIR="$REPO_ROOT/build"
APPDIR="$BUILD_DIR/AppDir"
INSTALL_PREFIX=/usr
APP_NAME=rtsm

# === CLEAN BUILD DIR ===
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
pushd "$BUILD_DIR"

# === CONFIGURE & BUILD IN RELEASE MODE ===
cmake "$REPO_ROOT" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_BUILD_TYPE=Release

make -j"$(nproc)"
make install DESTDIR="$APPDIR"

# === DOWNLOAD linuxdeploy AND PLUGIN ===
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
fi

if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
fi

chmod +x linuxdeploy*.AppImage

# === EXPORT QML PATH (adjust to match your project) ===
export QML_SOURCES_PATHS="$REPO_ROOT/ui/qt/qml"

# === CREATE APPIMAGE ===
./linuxdeploy-x86_64.AppImage \
    --appdir "$APPDIR" \
    --plugin qt \
    --output appimage

# === MOVE AppImage TO PROJECT ROOT ===
find . -name "*.AppImage" -exec mv {} "$REPO_ROOT" \;

popd
echo "✅ AppImage built and moved to $REPO_ROOT"

