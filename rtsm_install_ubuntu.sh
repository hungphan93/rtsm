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

APP_ICON="/opt/rtsm/icons"
ICON_NAME="app_icon.png"

WRAPPER_SCRIPT="$APP_DEST/AppRun"

# =====================
# Stop old service if exists
# =====================
systemctl --user stop "$SERVICE_NAME" 2>/dev/null || true
systemctl --user disable "$SERVICE_NAME" 2>/dev/null || true

# =====================
# Prepare directories + copy files
# =====================
echo "📦 Installing AppImage and icon..."
sudo mkdir -p "$APP_DEST" "$APP_ICON"
sudo cp "$APPIMAGE_NAME" "$APP_PATH"
sudo chmod +x "$APP_PATH"
sudo cp "./icons/$ICON_NAME" "$APP_ICON/"

# =====================
# Create AppRun wrapper (wait XWayland ready)
# =====================
echo "🛠️ Creating AppRun wrapper..."
sudo tee "$WRAPPER_SCRIPT" > /dev/null <<'EOF'
#!/usr/bin/env bash
# Wait until XWayland socket exists
XWAYLAND_SOCKET="/run/user/$UID/.X11-unix/X0"
i=0
while [ ! -S "$XWAYLAND_SOCKET" ] && [ $i -lt 5 ]; do
    sleep 0.5
    i=$((i+1))
done

# Environment variables
export QT_QPA_PLATFORM=xcb
export QT_AUTO_SCREEN_SCALE_FACTOR=1
export GDK_BACKEND=x11

exec /opt/rtsm/bin/RTSM-*x86_64.AppImage "$@"
EOF
sudo chmod +x "$WRAPPER_SCRIPT"

# =====================
# Grant sudo for dmidecode
# =====================
echo "🔐 Granting NOPASSWD for dmidecode..."
echo "$USER_NAME ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode" | sudo tee "$SUDOERS_FILE" > /dev/null
sudo chmod 440 "$SUDOERS_FILE"

# =====================
# Enable lingering (service runs after logout)
# =====================
sudo loginctl enable-linger "$USER_NAME"

# =====================
# Create systemd user service
# =====================
echo "🛠️ Creating systemd user service..."
mkdir -p "$SYSTEMD_USER_DIR"
tee "$SYSTEMD_USER_DIR/$SERVICE_NAME" > /dev/null <<EOF
[Unit]
Description=RTSM Realtime System Monitor
After=graphical-session.target dbus.service xdg-desktop-portal.service

[Service]
Type=simple
ExecStart=/bin/bash -c "sleep 5; $WRAPPER_SCRIPT"
Restart=always
RestartSec=5
Environment=QT_QPA_PLATFORM=xcb
Environment=QT_AUTO_SCREEN_SCALE_FACTOR=1

[Install]
WantedBy=graphical.target
EOF

# =====================
# Reload systemd user daemon and start service
# =====================
echo "🚀 Reloading systemd user daemon..."
export XDG_RUNTIME_DIR="/run/user/$(id -u $USER_NAME)"
export DBUS_SESSION_BUS_ADDRESS="unix:path=$XDG_RUNTIME_DIR/bus"

systemctl --user daemon-reexec
systemctl --user daemon-reload
systemctl --user enable --now "$SERVICE_NAME"

# =====================
# Create .desktop launcher
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
X-GNOME-Autostart-Delay=10
Comment=Realtime System Monitor
Categories=System;Utility;
StartupNotify=true
EOF
cp "$AUTOSTART_DIR/rtsm.desktop" "$APPLICATIONS_DIR/rtsm.desktop"

# =====================
# Done
# =====================
echo "🎉 RTSM installation complete. App will auto-start after login."

