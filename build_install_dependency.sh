#!/bin/bash

set -e
set -o pipefail

#!/bin/bash

# Create user "hungphan" with home directory if not exists
if ! id -u hungphan >/dev/null 2>&1; then
    echo "Creating user 'hungphan'..."
    useradd -m -s /bin/bash hungphan
else
    echo "User 'hungphan' already exists."
fi

# Ensure home directory exists
mkdir -p /home/hungphan

# Create subdirectories
mkdir -p /home/hungphan/Downloads
mkdir -p /home/hungphan/Qt
mkdir -p /home/hungphan/Documents

# Set ownership
chown -R hungphan:hungphan /home/hungphan

echo "✅ User and folders set up:"
ls -l /home/hungphan


echo "===> Installing required packages..."

apt update -qq && apt install -y -qq \
    build-essential \
    make \
    cmake \
    gcc \
    g++ \
    wget \
    unzip \
    tar \
    xz-utils \
    git \
    ninja-build \
    perl \
    python3 \
    git \
    libgl1-mesa-dev \
    libx11-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    libxrender-dev \
    libxcb1-dev \
    libxcb-glx0-dev \
    libxcb-keysyms1-dev \
    libxcb-image0-dev \
    libxcb-shm0-dev \
    libxcb-icccm4-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-shape0-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-xinerama0-dev \
    libxcb-util-dev \
    libxcb-cursor0 \
    libx11-xcb1 \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libfontconfig1-dev \
    libfreetype6-dev \
    libssl-dev \
    libpng-dev \
    libjpeg-dev \
    libglib2.0-dev \
    libdbus-1-dev \
    libpulse-dev \
    libasound2-dev > /dev/null
echo "✅ Dependencies installed."

# === Configuration ===
USER_HOME="/home/$USER"
QT_VERSION=6.9.0
QT_PREFIX="$USER_HOME/Qt/$QT_VERSION/gcc_64"
BUILD_DIR=build

# === Export Environment ===
export LD_LIBRARY_PATH=$QT_PREFIX/lib:$LD_LIBRARY_PATH
export PATH=$QT_PREFIX/bin:$PATH
export QT_PLUGIN_PATH=$QT_PREFIX/plugins
export QML2_IMPORT_PATH=$QT_PREFIX/qml

# === Environment Info ===
echo "===> Using Qt from: $QT_PREFIX"
echo "===> Home: $PWD"
echo "===> USER_HOME: $USER_HOME"
ls

# === Download and Extract Qt ===
cd ~
cd /home/$USER/Downloads/
echo "===> Downloading Qt.tar.xz..."
wget -q "https://drive.usercontent.google.com/download?id=1V7t8o21LFvt2BctjjoaOiqQZREqV5ExY&export=download&confirm=t&uuid=b4a6b4bb-ff0b-44f5-acce-4fcfd37c72ad" -O Qt.tar.xz

echo "===> Extracting Qt.tar.xz..."
mkdir -p "$USER_HOME/Qt"
ls
tar -xf Qt.tar.xz -C "$USER_HOME/"

# === Prepare and Build ===
echo "===> Preparing build directory..."
cd ~
mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo "===> Running CMake..."
cmake .. -DCMAKE_PREFIX_PATH="$QT_PREFIX/lib/cmake" -DCMAKE_BUILD_TYPE=Release > /dev/null

echo "===> Building project..."
cmake --build . --config Release -j$(nproc) > /dev/null

echo "✅ Build complete."

