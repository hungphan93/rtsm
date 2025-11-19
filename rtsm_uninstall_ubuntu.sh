#!/usr/bin/env bash
set -euo pipefail

# Define variables
USER_NAME="$USER"
HOME_DIR="/home/$USER_NAME"
SERVICE_NAME="rtsm.service"

# Stop the systemd user service if running
systemctl --user stop "$SERVICE_NAME" 2>/dev/null || true

# Disable the service to prevent start at login
systemctl --user disable "$SERVICE_NAME" 2>/dev/null || true

# Remove the service unit file from systemd user configuration
rm -f "$HOME_DIR/.config/systemd/user/$SERVICE_NAME"

# Reload systemd daemon to apply changes
systemctl --user daemon-reload
systemctl --user daemon-reexec

# Remove associated autostart launcher if exists
AUTOSTART_DIR="$HOME_DIR/.config/autostart"
DESKTOP_FILE="$AUTOSTART_DIR/rtsm.desktop"
rm -f "$DESKTOP_FILE"

# Remove any custom app files, icons, or scripts
APP_DEST="/opt/rtsm"
SUDOERS_FILE="/etc/sudoers.d/90-dmidecode-$USER_NAME"
sudo rm -rf "$APP_DEST"
sudo rm -f "$SUDOERS_FILE"
rm -rf "$HOME_DIR/.local/share/applications/rtsm.desktop"


# Optional: disable lingering if enabled
sudo loginctl disable-linger "$USER_NAME"

echo "RTSM systemd user service and related files have been removed."

# Kill remaining processes
pkill -f RTSM || true
pkill -f AppRun || true

echo "✅ RTSM uninstalled successfully."

