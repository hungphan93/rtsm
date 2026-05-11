#!/usr/bin/env bash
set -euo pipefail

QT_VERSION="6.11.0"
GCC_VERSION="15.2.0-native"

# ===== 1. Configuration & Context =====
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$REPO_ROOT/build"
TOOLS_DIR="$REPO_ROOT/builder_appimage"
APPDIR="$BUILD_DIR/AppDir"
OUTPUT_DIR="$REPO_ROOT/output_appimage"

# Architecture — currently x86_64 only. ARM (aarch64) support: TODO.
case "${ARCH:-$(uname -m)}" in
    x86_64) ARCH=x86_64; : "${QT_HOST_DIR:=gcc_64}" ;;
    *) echo "❌ Unsupported architecture: $(uname -m) (only x86_64 is supported for now)"; exit 1 ;;
esac
export ARCH QT_HOST_DIR
echo "🏗️  Building for arch=${ARCH} (Qt host dir: ${QT_HOST_DIR})"

# Export version for AppImage metadata (standard best practice)
export VERSION=$(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || echo "1.0.0")

# Git tag (or describe) used as suffix in the final AppImage filename
GIT_TAG=$(git -C "$REPO_ROOT" describe --tags --abbrev=0 2>/dev/null || echo "untagged")

# Fixed Tags for all tools for maximum stability
LINUXDEPLOY_TAG="1-alpha-20251107-1"
QT_PLUGIN_TAG="1-alpha-20250213-1"
RUNTIME_TAG="20251108"
APPIMAGETOOL_TAG="1.9.1"

# Download URLs based on specific release tags (arch-aware)
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/${LINUXDEPLOY_TAG}/linuxdeploy-${ARCH}.AppImage"
QT_PLUGIN_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/${QT_PLUGIN_TAG}/linuxdeploy-plugin-qt-${ARCH}.AppImage"
RUNTIME_URL="https://github.com/AppImage/type2-runtime/releases/download/${RUNTIME_TAG}/runtime-${ARCH}"
APPIMAGETOOL_URL="https://github.com/AppImage/appimagetool/releases/download/${APPIMAGETOOL_TAG}/appimagetool-${ARCH}.AppImage"

# Pinned SHA256 hashes — only listed for verified arches.
# For other arches the verification step is skipped with a warning.
declare -A TOOL_HASHES
TOOL_HASHES=(
    ["linuxdeploy-x86_64.AppImage"]="c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d"
    ["runtime-x86_64"]="2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d"
    ["appimagetool-x86_64.AppImage"]="ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0"
)

# ===== 2. Download & Hash Verification =====
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
    
    # Hash verification logic
    if [[ ${TOOL_HASHES[$file]:-""} != "" ]]; then
        # Format: [HASH]  [FILE] (Exactly 2 spaces required)
        echo "${TOOL_HASHES[$file]}  $file" | sha256sum --check --status || {
            echo "❌ Error: Hash mismatch for $file!"
            echo "Expected: ${TOOL_HASHES[$file]}"
            echo "Actual:   $(sha256sum "$file" | awk '{print $1}')"
            exit 1
        }
        echo "✅ $file: Verified."
    else
        echo "⚠️  $file: no pinned SHA256 for arch=${ARCH}, verification skipped."
    fi
}

download_tool "linuxdeploy-${ARCH}.AppImage" "$LINUXDEPLOY_URL"
download_tool "linuxdeploy-plugin-qt-${ARCH}.AppImage" "$QT_PLUGIN_URL"
download_tool "runtime-${ARCH}" "$RUNTIME_URL"
download_tool "appimagetool-${ARCH}.AppImage" "$APPIMAGETOOL_URL"
popd > /dev/null

# ===== 3. Project Compilation (RTSM) =====
# Clean environment for fresh build
rm -rf "$BUILD_DIR" && mkdir -p "$BUILD_DIR"
pushd "$REPO_ROOT" > /dev/null

# Inject CMake 4.3.0 to PATH (Required by project configuration)
export PATH="$HOME/Qt/Tools/CMake/bin:$PATH"

# Configure via preset (which sets GCC-15 environment and Qt paths).
# Override CMAKE_PREFIX_PATH for non-x86_64 hosts so the right Qt build is picked.
cmake --preset linux-gcc15-release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_PREFIX_PATH="$HOME/Qt/$QT_VERSION/$QT_HOST_DIR"

# Build via preset (automatically uses configured Ninja generator)
cmake --build --preset build-release -j"$(nproc)"

# Install directly to AppDir
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR/linux-gcc15-release"

# Organize Assets into standard AppDir layout
mkdir -p "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/256x256/apps"
cp "$REPO_ROOT/rtsm.desktop" "$APPDIR/usr/share/applications/rtsm.desktop"
cp "$REPO_ROOT/icons/app_icon.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/app_icon.png"
popd > /dev/null

# ===== 4. Final Packaging =====
echo "📦 Step 1: Bundling dependencies..."
export QML_SOURCES_PATHS="$REPO_ROOT/ui/qt/qml" # Critical for Qt dependencies

# Add GCC 15 and Qt paths so linuxdeploy resolves the correct dependencies instead of system ones
export PATH="$HOME/Qt/$QT_VERSION/$QT_HOST_DIR/bin:/opt/gcc/$GCC_VERSION/bin:$TOOLS_DIR:$PATH"
export LD_LIBRARY_PATH="$HOME/Qt/$QT_VERSION/$QT_HOST_DIR/lib:/opt/gcc/$GCC_VERSION/lib64:${LD_LIBRARY_PATH:-}"

# Prepare AppDir folder using linuxdeploy
"$TOOLS_DIR/linuxdeploy-${ARCH}.AppImage" \
    --appdir "$APPDIR" \
    --desktop-file "$APPDIR/usr/share/applications/rtsm.desktop" \
    --executable "$APPDIR/usr/bin/apprtsm" \
    --icon-file "$REPO_ROOT/icons/app_icon.png" \
    --plugin qt

echo "📦 Step 2: Generating final AppImage with verified runtime..."
rm -rf "$OUTPUT_DIR" && mkdir -p "$OUTPUT_DIR"
FINAL_FILE="$OUTPUT_DIR/RTSM-${ARCH}-${GIT_TAG}.AppImage"

# Manual packaging with specifically verified stable runtime
"$TOOLS_DIR/appimagetool-${ARCH}.AppImage" "$APPDIR" "$FINAL_FILE" --runtime-file "$TOOLS_DIR/runtime-${ARCH}"

echo "✅ Success! Final AppImage ready at: $FINAL_FILE"
