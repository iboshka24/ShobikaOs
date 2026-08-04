#!/bin/bash
# ShobikaOs - XFCE installation script
set -e

echo "[ShobikaOs] Installing XFCE desktop..."

# Base X11
pacman -S --noconfirm --needed \
    xorg-server \
    xorg-xinit \
    xdg-utils \
    xdg-user-dirs

# XFCE
pacman -S --noconfirm --needed \
    xfce4 \
    xfce4-goodies \
    xfce4-terminal \
    thunar \
    thunar-archive-plugin \
    thunar-volman \
    file-roller \
    ristretto \
    evince \
    mousepad \
    parole

# Display manager
pacman -S --noconfirm --needed lightdm lightdm-gtk-greeter
systemctl enable lightdm

# Enable services
systemctl enable NetworkManager

echo "[ShobikaOs] XFCE installed successfully!"
