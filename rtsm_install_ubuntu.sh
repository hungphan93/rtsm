#!/usr/bin/env bash
set -euo pipefail

# =====================
# Config
# =====================
USER_NAME="$USER"
USER_HOME="/home/$USER_NAME"

APPIMAGE_NAME=$(find output_appimage -maxdepth 1 -type f -name "RTSM-*x86_64.AppImage" | head -n1)
APP_DEST="/opt/rtsm/bin"
APP_PATH="$APP_DEST/$(basename "$APPIMAGE_NAME")"

AUTOSTART_DIR="$USER_HOME/.config/autostart"
APPLICATIONS_DIR="$USER_HOME/.local/share/applications"

APP_ICON="/opt/rtsm/icons"
ICON_NAME="app_icon.png"

WRAPPER_SCRIPT="$APP_DEST/AppRun"

# =====================
# Prepare directories + copy files
# =====================
echo "📦 Installing AppImage and icon..."
sudo mkdir -p "$APP_DEST" "$APP_ICON"
sudo cp "$APPIMAGE_NAME" "$APP_PATH"
sudo chmod +x "$APP_PATH"
sudo cp "./icons/$ICON_NAME" "$APP_ICON/"

# =====================
# Create AppRun wrapper (simplified)
# =====================
echo "🛠️ Creating AppRun wrapper..."
sudo tee "$WRAPPER_SCRIPT" > /dev/null <<EOF
#!/usr/bin/env bash

export QT_QPA_PLATFORM=xcb
export GDK_BACKEND=x11
export QT_SCALE_FACTOR_ROUNDING_POLICY=PassThrough

exec "$APP_PATH" "\$@"
EOF
sudo chmod +x "$WRAPPER_SCRIPT"

# =====================
# Grant sudo for dmidecode
# =====================
SUDOERS_FILE="/etc/sudoers.d/90-dmidecode-$USER_NAME"
echo "🔐 Granting NOPASSWD for dmidecode..."
echo "$USER_NAME ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode" | sudo tee "$SUDOERS_FILE" > /dev/null
sudo chmod 440 "$SUDOERS_FILE"

# =====================
# Create .desktop launcher for GNOME autostart
# =====================
echo "🖥️ Creating .desktop launcher..."
mkdir -p "$AUTOSTART_DIR" "$APPLICATIONS_DIR"
tee "$AUTOSTART_DIR/rtsm.desktop" > /dev/null <<EOF
[Desktop Entry]
Type=Application
Name=RTSM
Exec=$WRAPPER_SCRIPT
Icon=$APP_ICON/$ICON_NAME
Terminal=false
X-GNOME-Autostart-enabled=true
X-GNOME-Autostart-Delay=5
Comment=Realtime System Monitor
Categories=System;Utility;
StartupNotify=true
EOF

cp "$AUTOSTART_DIR/rtsm.desktop" "$APPLICATIONS_DIR/rtsm.desktop"

# =====================
# Done
# =====================
echo "🎉 RTSM installation complete. App will auto-start after GNOME login."

