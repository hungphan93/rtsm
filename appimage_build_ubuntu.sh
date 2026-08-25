#!/usr/bin/env bash
set -euo pipefail

# Parse Target Argument
TARGET="${1:-qt}" # Default to qt if no argument
if [[ ! "$TARGET" =~ ^(qt|cli|imgui|wasm)$ ]]; then
    echo "❌ Usage: $0 [qt|cli|imgui|wasm]"
    exit 1
fi

echo "🚀 Starting build for target: ${TARGET^^}"

# ===== 1. Configuration & Context =====
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$REPO_ROOT/build"
TOOLS_DIR="$REPO_ROOT/builder_appimage"
APPDIR="$BUILD_DIR/AppDir"
OUTPUT_DIR="$REPO_ROOT/output_appimage"

CMAKE_BIN="/opt/cmake/cmake-4.3.0/bin/cmake"

if [ "$TARGET" = "wasm" ]; then
    echo "🌐 Building WebAssembly with Emscripten..."
    mkdir -p "$BUILD_DIR/wasm-release"
    pushd "$BUILD_DIR/wasm-release" > /dev/null
    
    if ! command -v emcmake &> /dev/null; then
        echo "❌ Error: emcmake not found. Please activate EMSDK first."
        exit 1
    fi
    
    emcmake "$CMAKE_BIN" "$REPO_ROOT" -DCMAKE_BUILD_TYPE=Release
    "$CMAKE_BIN" --build . -j"$(nproc)" --target rtsm-imgui
    
    mkdir -p "$OUTPUT_DIR"
    find . -name "rtsm-imgui.html" -o -name "rtsm-imgui.js" -o -name "rtsm-imgui.wasm" -o -name "rtsm-imgui.data" -exec cp {} "$OUTPUT_DIR/" \;
    
    echo "✅ Success! WebAssembly output ready at: $OUTPUT_DIR"
    popd > /dev/null
    exit 0
fi

# Architecture (For AppImages)
case "${ARCH:-$(uname -m)}" in
    x86_64) ARCH=x86_64; QT_HOST_DIR="gcc_64" ;;
    *) echo "❌ Unsupported architecture: $(uname -m)"; exit 1 ;;
esac
export ARCH

# Export version for AppImage metadata
export VERSION=$(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || echo "1.0.0")
GIT_TAG=$(git -C "$REPO_ROOT" describe --tags --abbrev=0 2>/dev/null || echo "untagged")

# Required Tools Tags
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/linuxdeploy-${ARCH}.AppImage"
QT_PLUGIN_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/linuxdeploy-plugin-qt-${ARCH}.AppImage"
RUNTIME_URL="https://github.com/AppImage/type2-runtime/releases/download/20251108/runtime-${ARCH}"
APPIMAGETOOL_URL="https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-${ARCH}.AppImage"

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

if [ "$TARGET" = "qt" ]; then
    download_tool "linuxdeploy-plugin-qt-${ARCH}.AppImage" "$QT_PLUGIN_URL"
fi
popd > /dev/null

# ===== 3. Project Compilation =====
mkdir -p "$BUILD_DIR"
rm -rf "$APPDIR" "$BUILD_DIR/linux-gcc16-release"
pushd "$REPO_ROOT" > /dev/null

# CMake Preset already injects PATH, LD_LIBRARY_PATH, and CMAKE_PREFIX_PATH internally!
"$CMAKE_BIN" --preset linux-gcc16-release -DCMAKE_INSTALL_PREFIX=/usr

if [ "$TARGET" = "qt" ]; then
    "$CMAKE_BIN" --build --preset build-release -j"$(nproc)"
    DESTDIR="$APPDIR" "$CMAKE_BIN" --install "$BUILD_DIR/linux-gcc16-release" --strip
else
    "$CMAKE_BIN" --build --preset build-release -j"$(nproc)" --target "rtsm-$TARGET"
    DESTDIR="$APPDIR" "$CMAKE_BIN" --install "$BUILD_DIR/linux-gcc16-release" --component "rtsm-$TARGET" --strip
fi

# Organize Assets
mkdir -p "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/256x256/apps"
cp "$REPO_ROOT/icons/app_icon.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/app_icon.png"

# Setup target-specific variables for packaging
APP_EXEC="rtsm-${TARGET}"
APP_DESKTOP="rtsm-${TARGET}.desktop"
APP_NAME_SUFFIX="${TARGET^^}"

if [ "$TARGET" = "qt" ]; then
    APP_EXEC="apprtsm"
    APP_DESKTOP="rtsm.desktop"
elif [ "$TARGET" = "imgui" ]; then
    APP_NAME_SUFFIX="ImGui"
fi

cp "$REPO_ROOT/$APP_DESKTOP" "$APPDIR/usr/share/applications/$APP_DESKTOP"
popd > /dev/null

# ===== 4. Final Packaging =====
echo "📦 Step 1: Bundling dependencies..."

# Inject Qt and GCC paths so linuxdeploy resolves the correct dependencies
QT_VERSION="6.11.0"
GCC_VERSION="16.1.0-native"
export PATH="$HOME/Qt/$QT_VERSION/$QT_HOST_DIR/bin:/opt/gcc/$GCC_VERSION/bin:$TOOLS_DIR:$PATH"
export LD_LIBRARY_PATH="$HOME/Qt/$QT_VERSION/$QT_HOST_DIR/lib:/opt/gcc/$GCC_VERSION/lib64:${LD_LIBRARY_PATH:-}"

LINUXDEPLOY_ARGS=(
    --appdir "$APPDIR"
    --desktop-file "$APPDIR/usr/share/applications/$APP_DESKTOP"
    --executable "$APPDIR/usr/bin/$APP_EXEC"
    --icon-file "$REPO_ROOT/icons/app_icon.png"
)

if [ "$TARGET" = "qt" ]; then
    export QML_SOURCES_PATHS="$REPO_ROOT/rtsm-qt/ui/qt/qml"
    export EXTRA_QT_PLUGINS="wayland;xcb"
    LINUXDEPLOY_ARGS+=(--plugin qt)
fi

"$TOOLS_DIR/linuxdeploy-${ARCH}.AppImage" "${LINUXDEPLOY_ARGS[@]}"


echo "📂 Force-bundling GCC 16 runtime libraries..."
mkdir -p "$APPDIR/usr/lib"
cp -d /opt/gcc/$GCC_VERSION/lib64/libstdc++.so.6* "$APPDIR/usr/lib/"
cp -d /opt/gcc/$GCC_VERSION/lib64/libgcc_s.so* "$APPDIR/usr/lib/"

echo "📦 Step 2: Generating final AppImage..."
mkdir -p "$OUTPUT_DIR"
FINAL_FILE="$OUTPUT_DIR/RTSM-${APP_NAME_SUFFIX}-${ARCH}-${GIT_TAG}.AppImage"

"$TOOLS_DIR/appimagetool-${ARCH}.AppImage" "$APPDIR" "$FINAL_FILE" --runtime-file "$TOOLS_DIR/runtime-${ARCH}"

echo "✅ Success! Final AppImage ready at: $FINAL_FILE"
