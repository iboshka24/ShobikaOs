#!/bin/bash
# ShobikaOs - KDE Plasma installation script
set -e

echo "[ShobikaOs] Installing KDE Plasma desktop..."

# Base X11/Wayland
pacman -S --noconfirm --needed \
    xorg-server \
    xdg-utils \
    xdg-user-dirs

# KDE Plasma
pacman -S --noconfirm --needed \
    plasma \
    plasma-desktop \
    plasma-nm \
    plasma-pa \
    kde-applications \
    konsole \
    dolphin \
    ark \
    gwenview \
    okular \
    kcalc \
    kde-gtk-config \
    kvantum

# Display manager (SDDM)
pacman -S --noconfirm --needed sddm
systemctl enable sddm

# Enable services
systemctl enable NetworkManager

echo "[ShobikaOs] KDE Plasma installed successfully!"
