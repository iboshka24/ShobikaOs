#!/bin/bash
# ShobikaOs - Hyprland installation script
set -e

echo "[ShobikaOs] Installing Hyprland..."

# Hyprland + Wayland tools
pacman -S --noconfirm --needed \
    hyprland \
    hyprpaper \
    hyprlock \
    hypridle \
    waybar \
    wofi \
    wl-clipboard \
    grim \
    slurp \
    xdg-desktop-portal-hyprland \
    xdg-desktop-portal-gtk

# Terminal + file manager
pacman -S --noconfirm --needed \
    foot \
    thunar \
    thunar-archive-plugin \
    polkit-gnome \
    dunst \
    brightnessctl \
    pavucontrol \
    network-manager-applet \
    firefox

# Fonts
pacman -S --noconfirm --needed \
    ttf-font-awesome \
    ttf-jetbrains-mono-nerd

# Display manager
pacman -S --noconfirm --needed sddm
systemctl enable sddm

# Enable services
systemctl enable NetworkManager

echo "[ShobikaOs] Hyprland installed successfully!"
