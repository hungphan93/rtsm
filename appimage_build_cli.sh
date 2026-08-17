#!/usr/bin/env bash
set -euo pipefail

GCC_VERSION="16.1.0-native"

# ===== 1. Configuration & Context =====
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$REPO_ROOT/build"
TOOLS_DIR="$REPO_ROOT/builder_appimage"
APPDIR="$BUILD_DIR/AppDir"
OUTPUT_DIR="$REPO_ROOT/output_appimage"

# Architecture
case "${ARCH:-$(uname -m)}" in
    x86_64) ARCH=x86_64 ;;
    *) echo "❌ Unsupported architecture: $(uname -m)"; exit 1 ;;
esac
export ARCH

# Export version for AppImage metadata
export VERSION=$(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || echo "1.0.0")
GIT_TAG=$(git -C "$REPO_ROOT" describe --tags --abbrev=0 2>/dev/null || echo "untagged")

# Required Tools (Same as Qt version, but no Qt plugin needed)
LINUXDEPLOY_TAG="1-alpha-20251107-1"
RUNTIME_TAG="20251108"
APPIMAGETOOL_TAG="1.9.1"

LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_TAG}/linuxdeploy-${ARCH}.AppImage"
RUNTIME_URL="https://github.com/AppImage/type2-runtime/releases/download/${RUNTIME_TAG}/runtime-${ARCH}"
APPIMAGETOOL_URL="https://github.com/AppImage/appimagetool/releases/download/${APPIMAGETOOL_TAG}/appimagetool-${ARCH}.AppImage"

# ===== 2. Download Tools =====
mkdir -p "$TOOLS_DIR"
pushd "$TOOLS_DIR" > /dev/null

download_tool() {
    local file=$1
    local url=$2
    if [ ! -f "$file" ]; then
        echo "📥 Downloading $file..."
        wget -q --show-progress "$url" -O "$file"
        chmod +x "$file"
    fi
}

download_tool "linuxdeploy-${ARCH}.AppImage" "$LINUXDEPLOY_URL"
download_tool "runtime-${ARCH}" "$RUNTIME_URL"
download_tool "appimagetool-${ARCH}.AppImage" "$APPIMAGETOOL_URL"
popd > /dev/null

# ===== 3. Project Compilation (RTSM CLI) =====
rm -rf "$BUILD_DIR" && mkdir -p "$BUILD_DIR"
pushd "$REPO_ROOT" > /dev/null

export PATH="$HOME/Qt/Tools/CMake/bin:$PATH"

# Build using the same preset but we only care about CLI
cmake --preset linux-gcc16-release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build --preset build-release -j"$(nproc)" --target rtsm-cli

# Install CLI directly to AppDir
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR/linux-gcc16-release" --component rtsm-cli
popd > /dev/null

# Create a minimal desktop file for CLI
mkdir -p "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/256x256/apps"
cat <<EOF > "$APPDIR/usr/share/applications/rtsm-cli.desktop"
[Desktop Entry]
Name=RTSM CLI
Exec=rtsm-cli
Icon=app_icon
Type=Application
Terminal=true
Categories=System;Monitor;
EOF
cp "$REPO_ROOT/icons/app_icon.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/app_icon.png"

# ===== 4. Final Packaging =====
echo "📦 Bundling dependencies..."
export PATH="/opt/gcc/$GCC_VERSION/bin:$TOOLS_DIR:$PATH"
export LD_LIBRARY_PATH="/opt/gcc/$GCC_VERSION/lib64:${LD_LIBRARY_PATH:-}"

# Prepare AppDir folder using linuxdeploy (No Qt plugin needed)
"$TOOLS_DIR/linuxdeploy-${ARCH}.AppImage" \
    --appdir "$APPDIR" \
    --desktop-file "$APPDIR/usr/share/applications/rtsm-cli.desktop" \
    --executable "$APPDIR/usr/bin/rtsm-cli" \
    --icon-file "$REPO_ROOT/icons/app_icon.png"

echo "📂 Force-bundling GCC 16 runtime libraries..."
mkdir -p "$APPDIR/usr/lib"
cp -d /opt/gcc/$GCC_VERSION/lib64/libstdc++.so.6* "$APPDIR/usr/lib/"
cp -d /opt/gcc/$GCC_VERSION/lib64/libgcc_s.so* "$APPDIR/usr/lib/"

echo "📦 Generating final AppImage..."
mkdir -p "$OUTPUT_DIR"
FINAL_FILE="$OUTPUT_DIR/RTSM-CLI-${ARCH}-${GIT_TAG}.AppImage"

"$TOOLS_DIR/appimagetool-${ARCH}.AppImage" "$APPDIR" "$FINAL_FILE" --runtime-file "$TOOLS_DIR/runtime-${ARCH}"

echo "✅ Success! Final CLI AppImage ready at: $FINAL_FILE"
