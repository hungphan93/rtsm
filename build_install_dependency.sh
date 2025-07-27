#!/bin/bash

set -e  # Exit on error
set -o pipefail

apt update && apt install -y \
    build-essential \
    make \
    cmake \
    gcc \
    g++ \
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
    libasound2-dev

# === Configuration ===
echo "user home is $USER"
QT_VERSION=6.9.0
QT_PREFIX=/home/$USER/Qt/$QT_VERSION/gcc_64
BUILD_DIR=build

# === Export Environment ===
export LD_LIBRARY_PATH=$QT_PREFIX/lib:$LD_LIBRARY_PATH
export PATH=$QT_PREFIX/bin:$PATH
export QT_PLUGIN_PATH=$QT_PREFIX/plugins
export QML2_IMPORT_PATH=$QT_PREFIX/qml

# === Print environment info ===
echo "Using Qt from: $QT_PREFIX"
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo "QML2_IMPORT_PATH=$QML2_IMPORT_PATH"

# === Download qt and extract ===
echo "pwd is $PWD"
wget "https://drive.usercontent.google.com/download?id=1V7t8o21LFvt2BctjjoaOiqQZREqV5ExY&export=download&confirm=t&uuid=b4a6b4bb-ff0b-44f5-acce-4fcfd37c72ad" -O Qt.tar.xz

tar -xvf Qt.tar.xz -C /home/$USER/
echo "ls is $LS"


# === Prepare build directory ===
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# === Run CMake ===
cmake .. -DCMAKE_PREFIX_PATH=$QT_PREFIX/lib/cmake -DCMAKE_BUILD_TYPE=Release

# === Build Project ===
cmake --build . --config Release -j$(nproc)

