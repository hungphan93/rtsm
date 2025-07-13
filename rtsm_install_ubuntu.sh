#!/usr/bin/env bash
set -e

# Install binary
echo "Intalling  binary"
sudo mkdir -p /opt/rtsm/bin
sudo cp ./build/Desktop_x86-Release/apprtsm /opt/rtsm/bin/

# Install icon
echo "Intalling  icon"
sudo mkdir -p /opt/rtsm/icons
sudo cp ./icons/app_icon.png /opt/rtsm/icons/

# Install .desktop (user)
echo "Intalling  .desktop (user)"
mkdir -p ~/.local/share/applications
cat > ~/.local/share/applications/rtsm.desktop <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Rtsm
Comment=Monitor CPU, RAM, GPU in real time
Exec=/opt/rtsm/bin/apprtsm
Icon=/opt/rtsm/icons/app_icon.png
Terminal=false
Categories=Utility;System;
EOF

# Install systemd user service
echo "Intalling  systemd user service"
mkdir -p ~/.config/systemd/user
cat > ~/.config/systemd/user/rtsm.service <<EOF
[Unit]
Description=Real-time system monitor
After=graphical-session.target

[Service]
Type=simple
ExecStart=/opt/rtsm/bin/apprtsm
Restart=always
Environment=DISPLAY=:0
Environment=XAUTHORITY=%t/gdm/Xauthority

[Install]
WantedBy=graphical-session.target
EOF

# Enable the service
echo "Enable the service"
systemctl --user daemon-reload
systemctl --user enable --now rtsm.service
loginctl enable-linger $USER
systemctl --user start --now rtsm.service

echo "Done. App is installed and running."

