#!/usr/bin/env bash
set -e

echo "Stopping rtsm systemd user service..."
systemctl --user stop rtsm.service || true
systemctl --user disable rtsm.service || true
systemctl --user daemon-reload || true

echo "Removing systemd user service..."
rm -f ~/.config/systemd/user/rtsm.service

echo "Removing .desktop shortcut..."
rm -f ~/.local/share/applications/rtsm.desktop

echo "Removing installed files..."
sudo rm -rf /opt/rtsm
sudo rm -rf "/etc/sudoers.d/90-dmidecode-$USER_NAME"

echo "Reloading systemd user manager..."
systemctl --user daemon-reload

echo "Done. RTSM has been uninstalled."

