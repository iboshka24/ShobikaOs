# ShobikaOs — Modern Arch-based Linux Distribution

> **An elegant, high-performance Arch-based Linux distribution featuring a native Rust/GTK4 installer, automated hardware driver detection, PipeWire audio, out-of-the-box Gaming optimization, and Catppuccin Mocha aesthetics.**

[![Build ISO](https://github.com/iboshka24/ShobikaOs/actions/workflows/build-iso.yml/badge.svg)](https://github.com/iboshka24/ShobikaOs/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Fork of Arch Linux](https://img.shields.io/badge/Fork--Of-Arch%20Linux%20%2F%20Archiso-blue?logo=arch-linux)](https://github.com/archlinux/archiso)
[![Linux Kernel](https://img.shields.io/badge/Kernel-Linux%20Mainline-black?logo=linux)](https://github.com/torvalds/linux)
[![Rust](https://img.shields.io/badge/Installer-Rust%20%2B%20GTK4-orange?logo=rust)](https://www.rust-lang.org/)

---

## Upstream & Lineage

ShobikaOs is a derivative and custom distribution fork built upon:
- **Arch Linux / Archiso**: [github.com/archlinux/archiso](https://github.com/archlinux/archiso)
- **Linux Kernel**: [github.com/torvalds/linux](https://github.com/torvalds/linux)

---

## Key Features

- **Native Rust + GTK4 Installer**: Fast, safe 7-step installer with custom Catppuccin Mocha styling (`libadwaita` integrated).
- **Automated Hardware Driver Setup**:
  - **GPU**: Automatic detection & installation for **NVIDIA** (`nvidia-dkms`), **AMD** (`vulkan-radeon`), and **Intel** (`vulkan-intel`).
  - **CPU**: Auto-installs `intel-ucode` or `amd-ucode`.
  - **Peripherals**: Auto-configures **Bluetooth** (`bluez`), **Broadcom Wi-Fi**, and **Laptop power management** (`tlp`, `acpi`).
- **PipeWire Audio Stack**: Low-latency PipeWire, WirePlumber, and full codec bundle (`ffmpeg`, `gstreamer`, `libva`).
- **Gaming Stack Ready**: Pre-configured Steam, Lutris, Wine-Staging, GameMode, MangoHud, and VKD3D.
- **8 Catppuccin Mocha Wallpapers**: Choose your preferred wallpaper during installation (Default: *Northern Lights / Aurora*).
- **Deep System Tweaks**:
  - **BBR** TCP congestion control & sysctl kernel tuning (`vm.max_map_count` for gaming).
  - **zram** with zstd compression enabled by default.
  - **pacman**: 10 parallel downloads + `ILoveCandy` enabled.

---

## Building the ISO

### Option A: From CachyOS / Arch Linux (Recommended)
CachyOS is Arch-based, making it ideal for building ShobikaOs.

```bash
# 1. Update system & install dependencies
sudo pacman -Syu --needed archiso rust git base-devel

# 2. Clone repository
git clone https://github.com/iboshka24/ShobikaOs.git
cd ShobikaOs

# 3. Build Rust installer
cd installer
cargo build --release
cd ..

# 4. Copy installer binary into ISO root filesystem
cp installer/target/release/shobika-installer iso/airootfs/usr/bin/

# 5. Build ISO
sudo mkarchiso -v -w /tmp/shobikaiso -o out iso/
```

### Option B: From Tails (Debian-based)
Tails runs in RAM and is Debian-based. To build an Arch ISO inside Tails:
1. Boot Tails with an Admin Password.
2. Install `podman` or `docker` (or an Arch container) to run `archiso`:
```bash
sudo apt update && sudo apt install -y podman git
git clone https://github.com/iboshka24/ShobikaOs.git
cd ShobikaOs
# Build using Arch container
sudo podman run --rm --privileged -v $(pwd):/work archlinux:latest bash -c "
  pacman -Syu --noconfirm archiso rust git base-devel &&
  cd /work/installer && cargo build --release &&
  cp target/release/shobika-installer /work/iso/airootfs/usr/bin/ &&
  mkarchiso -v -w /tmp/shobikaiso -o /work/out /work/iso/
"
```

---

## Post-Install Script (`shobikaos-setup`)

After installing ShobikaOs, run the interactive post-install setup script:
```bash
shobikaos-setup
```
It handles:
- Installing `yay` (AUR helper)
- Enabling Flathub / Flatpak
- Generating reflector mirrorlist
- Quick install for VS Code, Spotify, Discord

---

## Contributing to Official Arch Linux & Linux Kernel

### How to contribute to Arch Linux (AUR & Trusted User / Dev):
1. **Maintain AUR Packages**: Create an account on [aur.archlinux.org](https://aur.archlinux.org/) and submit PKGBUILDs.
2. **Arch Wiki & GitLab**: Submit bug reports and merge requests to Arch's official GitLab: [gitlab.archlinux.org](https://gitlab.archlinux.org).
3. **Become an Arch Trusted User (TU)**: Maintain popular AUR packages and join the Arch community voting process.

### How to contribute to the Linux Kernel:
1. Clone the Linux kernel tree: `git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git`
2. Follow the kernel documentation on submitting patches via `git send-email`.
3. Test kernel builds using `make menuconfig && make -j$(nproc)`.

---

## License

Distributed under the MIT License. See `LICENSE` for details.