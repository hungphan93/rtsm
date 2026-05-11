# RTSM — Real-Time System Monitor

Lightweight C++23 + Qt6 QML system monitor. Clean Architecture: **Entity → UseCase → Adapter → Presenter → UI**.

Real-time **CPU**, **RAM**, **GPU**, **Disk**, **Network** with per-component sampling.

![RTSM demo](doc/demo_laptop_amd.gif)

---

## Toolchain

| Component | Version | Path |
|---|---|---|
| GCC       | ≥ 15.2.0 | `/opt/gcc/15.2.0-native` |
| CMake     | ≥ 4.3.0 | `/opt/cmake/cmake-4.3.0` |
| Qt        | 6.11.0  | `~/Qt/6.11.0/gcc_64` |
| Generator | Ninja   | — |
| Standard  | C++23 + Modules | — |

System packages (Debian/Ubuntu):

```bash
sudo apt install -y ninja-build git wget dmidecode libx11-dev
```

> Different install paths? Edit [`CMakePresets.json`](CMakePresets.json) and [`appimage_build.sh`](appimage_build.sh).

---

## Build a Release AppImage

```bash
git clone https://gitlab.com/hp210693/rtsm.git
cd rtsm
chmod +x appimage_build.sh
./appimage_build.sh
```

The script downloads pinned `linuxdeploy` / `appimagetool` (SHA256-verified), configures with preset `linux-gcc15-release`, builds, and packages.

**Supported architectures:** `x86_64` (ARM `aarch64` planned).

**Output:** `output_appimage/RTSM-<arch>-<git-tag>.AppImage`

### Manual build

```bash
cmake --preset linux-gcc15-release        # or linux-gcc15-debug
cmake --build --preset build-release -j"$(nproc)"
```

Binary: `build/linux-gcc15-release/apprtsm`

---

## Run

**1. Install the runtime dependency** (firmware info reader):

```bash
sudo apt install -y dmidecode
```

**2. Grant passwordless `dmidecode`** so RTSM can read firmware info without prompting:

```bash
sudo sh -c 'echo "'"$USER"' ALL=(ALL) NOPASSWD: /usr/sbin/dmidecode, /usr/bin/dmidecode" > /etc/sudoers.d/dmidecode'
sudo chmod 0440 /etc/sudoers.d/dmidecode
```

> Skip this step if you ran `rtsm_install_ubuntu.sh` — it configures sudoers automatically.

**3. Launch:**

```bash
chmod +x output_appimage/RTSM-<arch>-<git-tag>.AppImage
./output_appimage/RTSM-<arch>-<git-tag>.AppImage
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
entity/       Pure domain models                 (.ixx modules)
use_case/     Interactors + ports                (.ixx modules)
adapter/      OS-specific readers (/proc, /sys)  (.ixx modules)
presenter/    View-models                        (.ixx modules)
scheduler/    std::jthread periodic sampler
ui/qt/        Qt6 QML frontend + bindings
```

Dependencies flow inward: **UI → Presenter → UseCase → Entity**. Adapters implement UseCase ports.

---

## Troubleshooting

- **CMake/GCC not found** — fix `PATH` and `CC`/`CXX` in [`CMakePresets.json`](CMakePresets.json).
- **Qt not found** — fix `CMAKE_PREFIX_PATH` in [`CMakePresets.json`](CMakePresets.json).
- **AppImage hash mismatch** — delete `builder_appimage/` to re-download, or update hashes in [`appimage_build.sh`](appimage_build.sh).
- **GPU/firmware info missing** — install `dmidecode` and run `rtsm_install_ubuntu.sh` for sudoers.

---

## License

MIT — see [LICENSE](LICENSE).
