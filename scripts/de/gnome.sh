#!/bin/bash
# ShobikaOs - GNOME installation script
set -e

echo "[ShobikaOs] Installing GNOME desktop..."

# Base X11/Wayland
pacman -S --noconfirm --needed \
    xorg-server \
    xdg-utils \
    xdg-user-dirs

# GNOME
pacman -S --noconfirm --needed \
    gnome \
    gnome-extra \
    gnome-tweaks \
    gnome-terminal \
    nautilus \
    file-roller \
    eog \
    evince \
    gnome-calculator \
    gnome-font-viewer \
    firefox

# Display manager (GDM)
pacman -S --noconfirm --needed gdm
systemctl enable gdm

# Enable services
systemctl enable NetworkManager

echo "[ShobikaOs] GNOME installed successfully!"
