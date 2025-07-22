#!/bin/bash

set -ex

REPO_ROOT=$(readlink -f "$(dirname "$0")")
BUILD_DIR="$REPO_ROOT/build"
APPDIR="$BUILD_DIR/AppDir"
INSTALL_PREFIX=/usr
EXECUTABLE_PATH="$APPDIR/usr/bin/apprtsm"

DESKTOP_FILE="$APPDIR/usr/share/applications/rtsm.desktop"
ICON_TARGET="$APPDIR/usr/share/icons/hicolor/256x256/apps/app_icon.png"
ICON_SOURCE="$REPO_ROOT/icons/app_icon.png"

rm -rf "$BUILD_DIR"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$(dirname "$ICON_TARGET")"

pushd "$BUILD_DIR"

# === Build app ===
cmake "$REPO_ROOT" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"
make install DESTDIR="$APPDIR"

# === Install .desktop and icon ===
cp "$REPO_ROOT/rtsm.desktop" "$DESKTOP_FILE"
cp "$ICON_SOURCE" "$ICON_TARGET"

# === Download linuxdeployqt if needed ===
if [ ! -f linuxdeployqt-continuous-x86_64.AppImage ]; then
    wget -q https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
    chmod +x linuxdeployqt-continuous-x86_64.AppImage
fi

# === Export QML path if needed ===
export QML_SOURCES_PATHS="$REPO_ROOT/ui/qt/qml"

# === Create AppImage ===
./linuxdeployqt-continuous-x86_64.AppImage \
    "$DESKTOP_FILE" \
    -qmldir="$QML_SOURCES_PATHS" \
    -appimage

popd
APPIMAGE_FILE=$(find build -maxdepth 1 -type f -name "RTSM-*-x86_64.AppImage" -exec realpath {} \; | head -n 1)
echo "✅ AppImage created: $APPIMAGE_FILE"

