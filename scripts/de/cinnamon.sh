#!/bin/bash
# ShobikaOs - Cinnamon installation script
set -e

echo "[ShobikaOs] Installing Cinnamon desktop..."

# Base X11
pacman -S --noconfirm --needed \
    xorg-server \
    xorg-xinit \
    xorg-xrandr \
    xorg-xset \
    xdg-utils \
    xdg-user-dirs

# Cinnamon + apps
pacman -S --noconfirm --needed \
    cinnamon \
    cinnamon-wallpapers \
    cinnamon-control-center \
    cinnamon-screensaver \
    cinnamon-terminal \
    nemo \
    nemo-fileroller \
    nemo-image-converter \
    file-roller \
    eog \
    evince \
    gnome-calculator \
    gnome-screenshot \
    firefox

# Display manager
pacman -S --noconfirm --needed lightdm lightdm-gtk-greeter
systemctl enable lightdm

# Enable services
systemctl enable NetworkManager

echo "[ShobikaOs] Cinnamon installed successfully!"
