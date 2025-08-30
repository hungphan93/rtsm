#!/usr/bin/env bash
set -e

USER_NAME="$USER"
APPIMAGE_NAME=$(find build -maxdepth 1 -type f -name "RTSM-*-x86_64.AppImage" | head -n 1)
APP_DEST="/opt/rtsm/bin"
APP_PATH="$APP_DEST/$(basename "$APPIMAGE_NAME")"
SERVICE_NAME="rtsm.service"
SUDOERS_FILE="/etc/sudoers.d/90-dmidecode-$USER_NAME"
USER_HOME="/home/$USER_NAME"
SYSTEMD_USER_DIR="$USER_HOME/.config/systemd/user"
RUNTIME_DIR="/run/user/$(id -u "$USER_NAME")"
BUS_PATH="unix:path=$RUNTIME_DIR/bus"
AUTOSTART_DIR="$USER_HOME/.config/autostart"
APPLICATIONS_DIR="$USER_HOME/.local/share/applications"
DESKTOP_FILE_NAME="rtsm.desktop"
DESKTOP_FILE_PATH="$AUTOSTART_DIR/$DESKTOP_FILE_NAME"
APP_ICON="/opt/rtsm/icons"
ICON_NAME="app_icon.png"

# 0. Prepare AppImage directory
echo "📦 [0/7] Copying AppImage..."
sudo mkdir -p "$APP_DEST"
sudo cp "$APPIMAGE_NAME" "$APP_PATH"
sudo chmod +x "$APPIMAGE_NAME"

# 0. Prepare icon directory
echo "📦 [0/7] Intalling icon"
sudo mkdir -p $APP_ICON
sudo cp ./icons/$ICON_NAME $APP_ICON


# 1. Grant sudo for dmidecode
echo "🔐 [1/7] Granting NOPASSWD for dmidecode to $USER_NAME..."
echo "$USER_NAME ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode" | sudo tee "$SUDOERS_FILE" > /dev/null
sudo chmod 440 "$SUDOERS_FILE"

# 2. Enable lingering
echo "🔄 [2/7] Enabling systemd linger for $USER_NAME..."
sudo loginctl enable-linger "$USER_NAME"

# 3. Create systemd user service
echo "🛠️ [3/7] Creating user systemd service..."
sudo -u "$USER_NAME" mkdir -p "$SYSTEMD_USER_DIR"
sudo -u "$USER_NAME" tee "$SYSTEMD_USER_DIR/$SERVICE_NAME" > /dev/null <<EOF
[Unit]
Description=RTSM System Monitor App
After=graphical-session.target

[Service]
ExecStart=$APP_PATH
Restart=always
RestartSec=5

[Install]
WantedBy=default.target
EOF

# 4. Enable & start the service
echo "🚀 [4/7] Enabling and starting systemd service..."
sudo -u "$USER_NAME" XDG_RUNTIME_DIR="$RUNTIME_DIR" DBUS_SESSION_BUS_ADDRESS="$BUS_PATH" \
    systemctl --user daemon-reexec

sudo -u "$USER_NAME" XDG_RUNTIME_DIR="$RUNTIME_DIR" DBUS_SESSION_BUS_ADDRESS="$BUS_PATH" \
    systemctl --user daemon-reload

sudo -u "$USER_NAME" XDG_RUNTIME_DIR="$RUNTIME_DIR" DBUS_SESSION_BUS_ADDRESS="$BUS_PATH" \
    systemctl --user enable --now "$SERVICE_NAME"

# 5. Create .desktop file for autostart and menu integration
echo "🖥️ [5/7] Creating .desktop launcher and autostart..."
sudo -u "$USER_NAME" mkdir -p "$AUTOSTART_DIR" "$APPLICATIONS_DIR"
sudo -u "$USER_NAME" tee "$DESKTOP_FILE_PATH" > /dev/null <<EOF
[Desktop Entry]
Type=Application
Name=RTSM Monitor
Exec=$APP_PATH
Icon=$APP_ICON/$ICON_NAME
Terminal=false
X-GNOME-Autostart-enabled=true
Comment=Realtime System Monitor
Categories=System;Utility;
EOF
sudo -u "$USER_NAME" cp "$DESKTOP_FILE_PATH" "$APPLICATIONS_DIR/$DESKTOP_FILE_NAME"

# 6. Check service status
echo "✅ [6/7] Checking service status..."
sudo -u "$USER_NAME" XDG_RUNTIME_DIR="$RUNTIME_DIR" DBUS_SESSION_BUS_ADDRESS="$BUS_PATH" \
    systemctl --user status "$SERVICE_NAME" --no-pager

# 7. Done
echo "✅ [7/7] RTSM installed and configured for auto-start."

