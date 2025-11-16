#!/bin/bash

set -ex

REPO_ROOT=$(pwd)
BUILD_DIR="$REPO_ROOT/build"
APPDIR="$BUILD_DIR/AppDir"
INSTALL_PREFIX=/usr
EXECUTABLE_PATH="$APPDIR/usr/bin/apprtsm"
DESKTOP_FILE="$APPDIR/usr/share/applications/rtsm.desktop"
ICON_TARGET="$APPDIR/usr/share/icons/hicolor/256x256/apps/app_icon.png"
ICON_SOURCE="$REPO_ROOT/icons/app_icon.png"

echo "Removing /build folder"
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


APPIMAGE_TOOL_DIR="builder_appimage"
LINUXDEPLOY="linuxdeploy-x86_64.AppImage"
PLUGIN_QT="linuxdeploy-plugin-qt-x86_64.AppImage"
OUTPUT_DIR="output_appimage"

echo "Check if appimage_tool folder exists; if not, create it"
if [ ! -d "$APPIMAGE_TOOL_DIR" ]; then
	echo "Folder $APPIMAGE_TOOL_DIR does not exist. Creating..."
	cd ..
	mkdir -p "$OUTPUT_DIR"
	mkdir -p "$APPIMAGE_TOOL_DIR"
	cd "$APPIMAGE_TOOL_DIR"

        echo "Check if both files exist; if both exist, skip downloading"
	if [ -f "$LINUXDEPLOY" ] && [ -f "$PLUGIN_QT" ]; then
	        echo "Both $LINUXDEPLOY and $PLUGIN_QT already exist. No download needed."
	else
	        # Download linuxdeploy AppImage if missing
		if [ ! -f "$LINUXDEPLOY" ] || [ ! -f "$PLUGIN_QT" ]; then
		        echo "Downloading $LINUXDEPLOY..."
			wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/$LINUXDEPLOY -O "$LINUXDEPLOY"
			chmod +x "$LINUXDEPLOY"

                        echo "Downloading $PLUGIN_QT..."
			wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/$PLUGIN_QT -O "$PLUGIN_QT"
			chmod +x "$PLUGIN_QT"
		fi
		if [ ! -f "$LINUXDEPLOY" ] || [ ! -f "$PLUGIN_QT" ]; then
		        echo "Error: Failed to download $LINUXDEPLOY and $PLUGIN_QT. Stopping."
			exit 1
		else
		        echo "Downloaded $LINUXDEPLOY and $PLUGIN_QT."
		fi
	fi
	cd ..
else
        echo "Downloaded $LINUXDEPLOY and $PLUGIN_QT."
fi

cd "./build"
echo "Creating the AppImage file"
../"$APPIMAGE_TOOL_DIR"/"$LINUXDEPLOY" \
   --appdir AppDir \
   --desktop-file AppDir/usr/share/applications/rtsm.desktop \
   --executable AppDir/usr/bin/apprtsm \
   --icon-file "$ICON_SOURCE" \
   --plugin qt \
   --output appimage

popd

mv "./build/"RTSM-*.AppImage "./$OUTPUT_DIR"
find "$OUTPUT_DIR" -name "RTSM-*x86_64.AppImage" -exec mv {} "$OUTPUT_DIR/RTSM-x86_64.AppImage" \;
echo "✅ AppImage created. Check the $OUTPUT_DIR folder."
