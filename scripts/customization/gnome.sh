#!/bin/bash
# ShobikaOs - GNOME customization
set -e

echo "[ShobikaOs] Customizing GNOME..."

# Extensions
pacman -S --noconfirm --needed \
    gnome-shell-extension-dash-to-dock \
    gnome-shell-extension-appindicator

# Theme
gsettings set org.gnome.desktop.interface gtk-theme 'Arc-Dark'
gsettings set org.gnome.desktop.interface icon-theme 'Papirus-Dark'
gsettings set org.gnome.desktop.interface cursor-theme 'Adwaita'
gsettings set org.gnome.desktop.interface font-name 'Noto Sans 10'
gsettings set org.gnome.desktop.interface monospace-font-name 'JetBrains Mono 10'

# Dark mode
gsettings set org.gnome.desktop.interface color-scheme 'prefer-dark'

# Background
gsettings set org.gnome.desktop.background picture-uri 'file:///usr/share/backgrounds/shobikaos-wallpaper.png'
gsettings set org.gnome.desktop.background picture-uri-dark 'file:///usr/share/backgrounds/shobikaos-wallpaper.png'
gsettings set org.gnome.desktop.background picture-options 'zoom'

# Dock
gsettings set org.gnome.shell.extensions.dash-to-dock dock-position 'BOTTOM'
gsettings set org.gnome.shell.extensions.dash-to-dock dash-max-icon-size 48
gsettings set org.gnome.shell.extensions.dash-to-dock dock-fixed false

# Favorite apps
gsettings set org.gnome.shell favorite-apps "['firefox.desktop', 'org.gnome.Terminal.desktop', 'nautilus.desktop']"

echo "[ShobikaOs] GNOME customized!"
