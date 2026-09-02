# RTSM — Real-Time System Monitor

Lightweight C++26 + Qt6 QML system monitor with an optional CLI interface. Clean Architecture: **Entity → UseCase → Adapter → Presenter → UI**.

Real-time **CPU**, **RAM**, **GPU**, **Disk**, **Network** with per-component sampling.

![RTSM demo](doc/demo_laptop_amd.gif)

---

## Toolchain

| Component | Version | Path |
|---|---|---|
| GCC       | ≥ 16.2.0 | `/opt/gcc/16.2.0-native` |
| CMake     | ≥ 4.3.0 | `/opt/cmake/cmake-4.3.0` |
| Qt        | 6.11.2  | `~/Qt/6.11.2/gcc_64` |
| Generator | Ninja   | — |
| Standard  | C++26 + Modules | — |

System packages (Debian/Ubuntu):

```bash
sudo apt install -y ninja-build git wget dmidecode libx11-dev
```

> Different install paths? Export them in your environment, create a `CMakeUserPresets.json`, or edit [`appimage_build_ubuntu.sh`](appimage_build_ubuntu.sh).

---

## Build & Package (AppImage / WebAssembly)

The `appimage_build_ubuntu.sh` script automatically compiles the source code and packages it into a standalone AppImage (or WebAssembly files).

### Qt GUI AppImage
```bash
git clone https://gitlab.com/hp210693/rtsm.git
cd rtsm
chmod +x appimage_build_ubuntu.sh
./appimage_build_ubuntu.sh qt
```

### CLI AppImage
```bash
chmod +x appimage_build_ubuntu.sh
./appimage_build_ubuntu.sh cli
```

### Dear Imgui AppImage
```bash
chmod +x appimage_build_ubuntu.sh
./appimage_build_ubuntu.sh imgui
```

### WebAssembly (WASM) Build
```bash
chmod +x appimage_build_ubuntu.sh
./appimage_build_ubuntu.sh wasm
```

The scripts download pinned `linuxdeploy` / `appimagetool`, configure with preset `linux-release` (or `linux-gcc16-release` if you have user presets), build, and package.

**Supported architectures:** `x86_64` (ARM `aarch64` planned), `wasm`.

**Supported targets:** `QT` `CLI` `ImGui` `WASM`.

**Minimum System Requirement (AppImage):** `glibc >= 2.35` (e.g., Ubuntu 22.04 or newer).

**Output:** `output_appimage/RTSM-<TARGET>-<arch>-<git-tag>.AppImage` (for AppImage) or `output_appimage/` (for WebAssembly)

## Manual Build (Binaries Only)

If you only want to compile the raw executables without packaging them into an AppImage, you can use CMake directly:

```bash
# Release build
cmake --preset linux-release
cmake --build --preset build-release -j"$(nproc)"

# Debug build
cmake --preset linux-debug
cmake --build --preset build-debug -j"$(nproc)"
```

Binaries: 
- Qt GUI: `build/linux-release/rtsm-qt/apprtsm`
- CLI: `build/linux-release/rtsm-cli/rtsm-cli`
- ImGui: `build/linux-release/rtsm-imgui/rtsm-imgui`

---

## Run

**1. Install the runtime dependency** (firmware info reader):

```bash
sudo apt install -y dmidecode
```

**2. Grant passwordless `dmidecode`** so RTSM can read firmware info without prompting:

```bash
echo "$USER ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode" | sudo tee /etc/sudoers.d/90-dmidecode-$USER
sudo chmod 440 /etc/sudoers.d/90-dmidecode-$USER
```

> Skip this step if you ran `rtsm_install_ubuntu.sh` — it configures sudoers automatically.

**3. Launch:**

```bash
chmod +x output_appimage/RTSM-<TARGET>-<arch>-<git-tag>.AppImage
./output_appimage/RTSM-<TARGET>-<arch>-<git-tag>.AppImage
```

---

## Install / Uninstall (Ubuntu, Debian, GNOME)

```bash
./rtsm_install_ubuntu.sh     # installs to /opt/rtsm, adds GNOME autostart, sudoers for dmidecode
./rtsm_uninstall_ubuntu.sh   # removes everything
```

---

## Project Layout

```
rtsm-core/    Core logic (Entity, UseCase, Adapter, Presenter, Scheduler)
rtsm-qt/      Qt6 QML frontend + bindings
rtsm-cli/     Command-line interface
rtsm-imgui/   Dear ImGui frontend
```

Dependencies flow inward: **UI → Presenter → UseCase → Entity**. Adapters implement UseCase ports.

---

## Troubleshooting

- **CMake/GCC not found** — export `CC`/`CXX` in your environment or define them in `CMakeUserPresets.json`.
- **Qt not found** — export `CMAKE_PREFIX_PATH` in your environment or define it in `CMakeUserPresets.json`.
- **AppImage download fails** — delete `builder_appimage/` to re-download.
- **GPU/firmware info missing** — install `dmidecode` and run `rtsm_install_ubuntu.sh` for sudoers.

---

## License

MIT — see [LICENSE](LICENSE).
