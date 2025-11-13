#!/usr/bin/env bash
set -euo pipefail

# =====================
# Config
# =====================
USER_NAME="$USER"
USER_HOME="/home/$USER_NAME"

APPIMAGE_NAME=$(find build -maxdepth 1 -type f -name "RTSM-*x86_64.AppImage" | head -n1)
APP_DEST="/opt/rtsm/bin"
APP_PATH="$APP_DEST/$(basename "$APPIMAGE_NAME")"

SERVICE_NAME="rtsm.service"
SUDOERS_FILE="/etc/sudoers.d/90-dmidecode-$USER_NAME"

SYSTEMD_USER_DIR="$USER_HOME/.config/systemd/user"
AUTOSTART_DIR="$USER_HOME/.config/autostart"
APPLICATIONS_DIR="$USER_HOME/.local/share/applications"
DESKTOP_FILE_NAME="rtsm.desktop"
DESKTOP_FILE_PATH="$AUTOSTART_DIR/$DESKTOP_FILE_NAME"

APP_ICON="/opt/rtsm/icons"
ICON_NAME="app_icon.png"

WRAPPER_SCRIPT="$APP_DEST/AppRun"

# =====================
# 0. Prepare directories + copy files
# =====================
echo "📦 [0/7] Installing AppImage and icon..."
sudo mkdir -p "$APP_DEST" "$APP_ICON"
sudo cp "$APPIMAGE_NAME" "$APP_PATH"
sudo chmod +x "$APP_PATH"
sudo cp "./icons/$ICON_NAME" "$APP_ICON/"

# =====================
# 0.1 Create AppRun wrapper
# =====================
echo "🛠️ [0.1/7] Creating AppRun wrapper..."
sudo tee "$WRAPPER_SCRIPT" > /dev/null <<'EOF'
#!/usr/bin/env bash
export QT_QPA_PLATFORM=xcb
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export QT_SCREEN_SCALE_FACTORS=1
sleep 5

exec /opt/rtsm/bin/RTSM-*x86_64.AppImage --appimage-extract-and-run "$@"
EOF
sudo chmod +x "$WRAPPER_SCRIPT"

# =====================
# 1. Grant sudo for dmidecode
# =====================
echo "🔐 [1/7] Granting NOPASSWD for dmidecode..."
echo "$USER_NAME ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode" | sudo tee "$SUDOERS_FILE" > /dev/null
sudo chmod 440 "$SUDOERS_FILE"

# =====================
# 2. Enable lingering
# =====================
echo "🔄 [2/7] Enabling systemd linger..."
sudo loginctl enable-linger "$USER_NAME"

# =====================
# 3. Create systemd user service
# =====================
echo "🛠️ [3/7] Creating systemd user service..."
mkdir -p "$SYSTEMD_USER_DIR"
tee "$SYSTEMD_USER_DIR/$SERVICE_NAME" > /dev/null <<EOF
[Unit]
Description=RTSM Realtime System Monitor
After=graphical-session.target

[Service]
ExecStart=/bin/bash -c $WRAPPER_SCRIPT
Restart=always
RestartSec=5

[Install]
WantedBy=default.target
EOF

# =====================
# 4. Reload systemd user daemon (no sudo)
# =====================
echo "🚀 [4/7] Reloading systemd user daemon..."
export XDG_RUNTIME_DIR="/run/user/$(id -u $USER_NAME)"
export DBUS_SESSION_BUS_ADDRESS="unix:path=$XDG_RUNTIME_DIR/bus"

systemctl --user daemon-reexec
systemctl --user daemon-reload
systemctl --user enable --now "$SERVICE_NAME"

# =====================
# 5. Create .desktop file for autostart + menu
# =====================
echo "🖥️ [5/7] Creating .desktop launcher..."
mkdir -p "$AUTOSTART_DIR" "$APPLICATIONS_DIR"
tee "$DESKTOP_FILE_PATH" > /dev/null <<EOF
[Desktop Entry]
Type=Application
Name=RTSM
Exec=$WRAPPER_SCRIPT
Icon=$APP_ICON/$ICON_NAME
Terminal=false
X-GNOME-Autostart-enabled=true
Comment=Realtime System Monitor
Categories=System;Utility;
StartupNotify=true
EOF
cp "$DESKTOP_FILE_PATH" "$APPLICATIONS_DIR/$DESKTOP_FILE_NAME"

# =====================
# 6. Check service status
# =====================
echo "✅ [6/7] Checking systemd service status..."
systemctl --user status "$SERVICE_NAME" --no-pager

# =====================
# 7. Done
# =====================
echo "🎉 [7/7] RTSM installation complete. App will auto-start after login."

