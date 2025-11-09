#!/bin/bash
# Build AppImage thủ công bằng appimagetool (NO linuxdeployqt / linuxdeploy)
set -euo pipefail
set -x

########################
#       CONFIG         #
########################
REPO_ROOT=$(readlink -f "$(dirname "$0")")
BUILD_DIR="$REPO_ROOT/build"
APPDIR="$BUILD_DIR/AppDir"
INSTALL_PREFIX=/usr

APP_NAME="RTSM"
APP_ID="rtsm"          # trùng tên .desktop
BIN_NAME="apprtsm"     # tên binary sau install

DESKTOP_SRC="$REPO_ROOT/rtsm.desktop"
ICON_SRC="$REPO_ROOT/icons/app_icon.png"
QML_SRC="$REPO_ROOT/ui/qt/qml"

# Qt cục bộ (Qt 6.8 → cần ICU 73). Sửa nếu bạn cài chỗ khác.
QT_HOME="${QT_HOME:-/home/hungphan/Qt/6.9.1/gcc_64}"
QMAKE="$QT_HOME/bin/qmake"

########################
#   HELPERS            #
########################
have() { command -v "$1" >/dev/null 2>&1; }
copy_glob_if_exists(){ compgen -G "$1" >/dev/null 2>&1 && cp $1 "$2" || true; }
copy_file_if_exists(){ [ -f "$1" ] && install -m755 "$1" "$2" || true; }

copy_deps() {
  local obj="$1"
  ldd "$obj" | awk '{print $1 " " $3}' | while read -r name path; do
    [ "$name" = "statically" ] && continue
    [ "$name" = "not" ] && continue
    case "$name" in
      linux-vdso.so.*|ld-linux-x86-64.so.*|libc.so.*|libm.so.*|libdl.so.*|libpthread.so.*|librt.so.*|libresolv.so.*|libnsl.so.*) continue ;;
      libX11.so.*|libXext.so.*|libxcb.so.*|libXrender.so.*|libXfixes.so.*|libXi.so.*|libGL.so.*) continue ;;
    esac
    if [ -n "${path:-}" ] && [ -f "$path" ]; then
      install -m755 "$path" "$APPDIR/usr/lib/x86_64-linux-gnu/" || true
    fi
  done
}

rsync_copy(){ if have rsync; then rsync -a "$1" "$2"; else cp -a "$1" "$2"; fi; }

# Lấy số phiên bản ICU SONAME mà binary/lib Qt yêu cầu (vd 73)
detect_required_icu_major() {
  { ldd "$APPDIR/usr/bin/$BIN_NAME"; ldd "$APPDIR/usr/lib"/libQt6*.so.* 2>/dev/null; } \
    | awk '/libicu/{print $1}' | sed -n 's/^libicu[^.]*\.so\.\([0-9][0-9]*\)$/\1/p' \
    | sort -u | tail -n1
}

# Vendor ICU đúng phiên bản (ưu tiên từ Qt, fallback Docker ubuntu:23.10 nếu cần 73)
vendor_icu_matching() {
  local need_major="$1"
  mkdir -p "$APPDIR/usr/lib/x86_64-linux-gnu"

  # 1) Nếu Qt có sẵn ICU → copy luôn
  if compgen -G "$QT_HOME/lib/libicu*.so*" >/dev/null 2>&1; then
    cp "$QT_HOME/lib/"libicu*.so* "$APPDIR/usr/lib/x86_64-linux-gnu/" || true
    return 0
  fi

  # 2) Thử tìm trên hệ
  local ok=1
  for soname in "libicui18n.so.$need_major" "libicuuc.so.$need_major" "libicudata.so.$need_major"; do
    real=$(ldconfig -p | awk -v n="$soname" '$1==n{print $NF; exit}')
    if [ -n "$real" ] && [ -f "$real" ]; then
      install -m755 "$real" "$APPDIR/usr/lib/x86_64-linux-gnu/" || true
    else
      ok=0
    fi
  done
  [ "$ok" -eq 1 ] && return 0

  # 3) Fallback: nếu cần 73 và có Docker → trích từ ubuntu:23.10
  if [ "$need_major" = "73" ] && have docker; then
    echo "→ Dùng Docker để trích ICU 73 từ ubuntu:23.10"
    docker pull ubuntu:23.10
    cid=$(docker create ubuntu:23.10 bash -lc "apt-get update && apt-get install -y --no-install-recommends libicu73 && sleep 1")
    docker start "$cid" >/dev/null
    mkdir -p "$BUILD_DIR/icu73"
    docker cp "$cid":/usr/lib/x86_64-linux-gnu/libicui18n.so.73 "$BUILD_DIR/icu73/" || true
    docker cp "$cid":/usr/lib/x86_64-linux-gnu/libicuuc.so.73   "$BUILD_DIR/icu73/" || true
    docker cp "$cid":/usr/lib/x86_64-linux-gnu/libicudata.so.73 "$BUILD_DIR/icu73/" || true
    docker rm -f "$cid" >/dev/null || true
    copy_glob_if_exists "$BUILD_DIR/icu73/libicu*.so.73" "$APPDIR/usr/lib/x86_64-linux-gnu/"
    return 0
  fi

  echo "⚠️  Không tự động lấy được ICU $need_major."
  echo "   • Cách nhanh: cài Docker rồi chạy lại script (sẽ tự trích ICU $need_major)."
  echo "   • Hoặc rebuild app với Qt hệ (Ubuntu 22.04 → ICU 70) để không cần ICU $need_major."
  return 0
}

########################
#   PREPARE FOLDERS    #
########################
rm -rf "$BUILD_DIR"
mkdir -p "$APPDIR/usr/share/applications" \
         "$APPDIR/usr/share/icons/hicolor/256x256/apps" \
         "$APPDIR/usr/lib" \
         "$APPDIR/usr/lib/x86_64-linux-gnu" \
         "$APPDIR/usr/plugins" \
         "$APPDIR/usr/qml"

########################
#   BUILD & INSTALL    #
########################
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
  -DCMAKE_C_COMPILER=/usr/bin/gcc \
  -DCMAKE_CXX_COMPILER=/usr/bin/g++

cmake --build "$BUILD_DIR" -j"$(nproc)"
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR" --prefix "$INSTALL_PREFIX"

# .desktop + icon (đặt thêm ở root để appimagetool pick up)
install -Dm644 "$DESKTOP_SRC" "$APPDIR/usr/share/applications/$APP_ID.desktop"
install -Dm644 "$ICON_SRC"    "$APPDIR/usr/share/icons/hicolor/256x256/apps/app_icon.png"
cp "$APPDIR/usr/share/applications/$APP_ID.desktop" "$APPDIR/$APP_ID.desktop"
cp "$APPDIR/usr/share/icons/hicolor/256x256/apps/app_icon.png" "$APPDIR/app_icon.png"

########################
#       AppRun         #
########################
cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="$HERE/usr/lib:$HERE/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="$HERE/usr/plugins"
export QML2_IMPORT_PATH="$HERE/usr/qml"
exec "$HERE/usr/bin/apprtsm" "$@"
EOF
chmod +x "$APPDIR/AppRun"

########################
#       qt.conf        #
########################
mkdir -p "$APPDIR/usr/bin"
cat > "$APPDIR/usr/bin/qt.conf" <<'EOF'
[Paths]
Plugins = ../plugins
Qml2Imports = ../qml
EOF

########################
#     QT PLUGINS       #
########################
PLUGINS_DIR=""
if [ -x "$QMAKE" ]; then
  PLUGINS_DIR="$("$QMAKE" -query QT_INSTALL_PLUGINS)"
fi
mkdir -p "$APPDIR/usr/plugins/platforms" "$APPDIR/usr/plugins/imageformats" "$APPDIR/usr/plugins/sqldrivers"
[ -n "$PLUGINS_DIR" ] && copy_file_if_exists "$PLUGINS_DIR/platforms/libqxcb.so" "$APPDIR/usr/plugins/platforms/"
[ -n "$PLUGINS_DIR" ] && copy_glob_if_exists "$PLUGINS_DIR/imageformats/libq*.so" "$APPDIR/usr/plugins/imageformats/"
[ -n "$PLUGINS_DIR" ] && copy_file_if_exists "$PLUGINS_DIR/sqldrivers/libqsqlite.so" "$APPDIR/usr/plugins/sqldrivers/"

########################
#         QML          #
########################
if [ -d "$QML_SRC" ]; then
  mkdir -p "$APPDIR/usr/qml/app"
  rsync_copy "$QML_SRC/" "$APPDIR/usr/qml/app/"
fi
if [ -x "$QMAKE" ]; then
  QT_QML_DIR="$("$QMAKE" -query QT_INSTALL_QML)"
  for mod in QtQml QtQuick QtQuick/Layouts QtQuick/Controls QtQuick/Window QtQuick/Controls/Material QtCore QtCore5Compat; do
    [ -d "$QT_QML_DIR/$mod" ] || continue
    mkdir -p "$APPDIR/usr/qml/$(dirname "$mod")"
    rsync_copy "$QT_QML_DIR/$mod" "$APPDIR/usr/qml/$(dirname "$mod")/../"
  done
fi

########################
#    QT6 RUNTIME SO    #
########################
copy_glob_if_exists "$QT_HOME/lib/libQt6Core.so.*"              "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6Gui.so.*"               "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6Network.so.*"           "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6DBus.so.*"              "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6OpenGL.so.*"            "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6Concurrent.so.*"        "$APPDIR/usr/lib/"

# QML/Quick stack + QmlMeta (rất quan trọng với Qt 6.7+)
copy_glob_if_exists "$QT_HOME/lib/libQt6Qml.so.*"               "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6QmlModels.so.*"         "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6QmlWorkerScript.so.*"   "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6QmlMeta.so.*"           "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6Quick.so.*"             "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6QuickLayouts.so.*"      "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6QuickTemplates2.so.*"   "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6QuickControls2.so.*"    "$APPDIR/usr/lib/"
copy_glob_if_exists "$QT_HOME/lib/libQt6Core5Compat.so.*"       "$APPDIR/usr/lib/"

########################
#      PULL DEPS       #
########################
copy_deps "$APPDIR/usr/bin/$BIN_NAME"
find "$APPDIR/usr/plugins" -type f -name "*.so" -print0 | while IFS= read -r -d '' so; do copy_deps "$so"; done
find "$APPDIR/usr/qml"     -type f -name "*.so" -print0 | while IFS= read -r -d '' so; do copy_deps "$so"; done
find "$APPDIR/usr/lib"     -maxdepth 1 -type f -name "libQt6*.so.*" -print0 | while IFS= read -r -d '' so; do copy_deps "$so"; done

########################
#     ICU MATCHING     #
########################
REQ_ICU_MAJOR="$(detect_required_icu_major || true)"
if [ -n "${REQ_ICU_MAJOR:-}" ]; then
  echo "→ ICU required major: ${REQ_ICU_MAJOR}"
  vendor_icu_matching "$REQ_ICU_MAJOR"
fi

# Một số phụ thuộc hay thiếu
copy_glob_if_exists "/usr/lib/x86_64-linux-gnu/libxkbcommon*.so.*" "$APPDIR/usr/lib/x86_64-linux-gnu/"
copy_glob_if_exists "/usr/lib/x86_64-linux-gnu/libzstd.so.*"       "$APPDIR/usr/lib/x86_64-linux-gnu/"
copy_glob_if_exists "/lib/x86_64-linux-gnu/libgcrypt.so.*"         "$APPDIR/usr/lib/x86_64-linux-gnu/"
copy_glob_if_exists "/lib/x86_64-linux-gnu/libgpg-error.so.*"      "$APPDIR/usr/lib/x86_64-linux-gnu/"
copy_glob_if_exists "/lib/x86_64-linux-gnu/libmd.so.*"             "$APPDIR/usr/lib/x86_64-linux-gnu/"
copy_glob_if_exists "/usr/lib/x86_64-linux-gnu/libstdc++.so.6*"    "$APPDIR/usr/lib/x86_64-linux-gnu/"  # tránh kéo /usr/local/gcc-13

########################
#        RPATH         #
########################
if have patchelf; then
  patchelf --set-rpath '$ORIGIN:$ORIGIN/../lib/x86_64-linux-gnu:$ORIGIN/../lib' \
    "$APPDIR/usr/bin/$BIN_NAME" || true
fi

########################
#     LDD PRE-CHECK    #
########################
echo "== LDD check =="
ldd "$APPDIR/usr/bin/$BIN_NAME" | grep "not found" && { echo "❌ Thiếu deps cho binary"; exit 1; } || echo "Binary OK"
for so in "$APPDIR/usr/lib/libQt6QmlMeta.so."* "$APPDIR/usr/lib/libQt6Qml.so."* "$APPDIR/usr/lib/libQt6Quick.so."*; do
  [ -f "$so" ] || continue
  ldd "$so" | grep "not found" && { echo "❌ Thiếu deps cho $(basename "$so")"; exit 1; } || echo "$(basename "$so") OK"
done

########################
#    APPIMAGETOOL      #
########################
APPIMAGETOOL="$BUILD_DIR/appimagetool-x86_64.AppImage"
if [ ! -f "$APPIMAGETOOL" ]; then
  wget -q -O "$APPIMAGETOOL" https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
  chmod +x "$APPIMAGETOOL"
fi

( cd "$BUILD_DIR" && ./appimagetool-x86_64.AppImage AppDir "${APP_NAME}-x86_64.AppImage" )
echo "✅ AppImage: $BUILD_DIR/${APP_NAME}-x86_64.AppImage"
echo "👉 Run: chmod +x '$BUILD_DIR/${APP_NAME}-x86_64.AppImage' && '$BUILD_DIR/${APP_NAME}-x86_64.AppImage'"

