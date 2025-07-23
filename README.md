# ⚡ RTSM - Real-Time System Monitor

**RTSM** is a lightweight, modular system monitor built in **C++20** using **Qt6**, following **Clean Architecture** principles. It displays real-time **CPU**, **RAM**, **GPU**, **Disk**, **Network** infomation.

---

## 🌟 Features

- cross-plass from (currently linux)
- 🔥 Real-time CPU usage (%), frequency, core info, and temperature  
- 🧠 RAM usage, frequency, and temperature  
- 🎮 GPU usage, frequency, and temperature  
- 💾 Disk usage and device info  
- 🌐 Network upload/download speeds  
- ⏱️ Individual update intervals per component  
- 🧵 Thread-safe fetching using `std::jthread`  
- 🎨 Qt6 QML UI with C++ backend via `Q_PROPERTY`  
- 🧼 Clean Architecture: **Entity → UseCase → Adapter → Presenter → UI**

---

## ⚙️ Requirements

- 🐧 Ubuntu 22.04.5 LTS  
- 📦 Qt 6.9.1  
- 🛠️ GCC 11.4.0 or compatible with C++23  
- 🧱 CMake ≥ 3.16  
- 🔍 `dmidecode` (`sudo apt install dmidecode`)  
- 📦 `linuxdeployqt` (for AppImage creation)

---

## 📦 Build & Package as AppImage 

1. **Clone the Repository**
    ```bash
    git clone https://gitlab.com/hp210693/rtsm.git
    cd rtsm
    ```

2. **Make the Build Script Executable**
    ```bash
    chmod +x appimage_build.sh
    ```

3. **Run the Build Script**
    ```bash
    ./appimage_build.sh
    ```

The output will be an AppImage file in the build directory, typically named:  
`RTSM-<build-commit-hash>-x86_64.AppImage`

---

### 🚀 Autostart Setup

1. **Make AppImage Executable**
    ```bash
    chmod +x RTSM-*-x86_64.AppImage
    ```

2. **Run the Application**
    ```bash
    ./RTSM-*-x86_64.AppImage
    ```

---

### 🧼 Uninstall
1. **Make AppImage Executable**
    ```bash
    chmod +x rtsm_uninstall_ubuntu.sh 
    ```
    
2. **Remove the Application**
    ```bash
    ./rtsm_uninstall_ubuntu.sh
    
    ```

## 📁 Project Structure

rtsm/  
├── entity/ # Data model: cpu, ram, gpu, etc.  
├── usecase/ # Abstract reader interfaces  
├── adapter/ # OS-specific system info fetchers  
├── presenter/ # Scheduler & data access for UI  
├── ui_qt/ # Qt6 QML frontend + bindings  
├── appimage_build.sh # create .AppImage file  
├── install_rtsm.sh  # install app  
├── rtsm.desktop # Autostart launcher entry  
├── rtsm-icon.png # Desktop icon  
└── README.md # This file  

---

## 📄 License

This project is licensed under the **MIT License**.  
See the [LICENSE](LICENSE) file for details.

---

