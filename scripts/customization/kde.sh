#!/bin/bash
# ShobikaOs - KDE Plasma customization
set -e

echo "[ShobikaOs] Customizing KDE Plasma..."

# Kvantum theme
pacman -S --noconfirm --needed kvantum

# Set theme
cat > ~/.config/kdeglobals << 'EOF'
[General]
Name=ShobikaOs
Theme=Breeze Dark
ColorScheme=ShobikaOs

[Icons]
Theme=Papirus-Dark
EOF

# Plasma theme
cat > ~/.config/plasmarc << 'EOF'
[Theme]
name=breeze-dark
EOF

# Wallpaper
cp /usr/share/backgrounds/shobikaos-wallpaper.png ~/.local/share/wallpapers/shobikaos-wallpaper.png

# Kvantum
mkdir -p ~/.config/Kvantum
cat > ~/.config/Kvantum/kvantum.kvconfig << 'EOF'
[General]
theme=kvantum-dark
EOF

# Panel layout
cat > ~/.config/plasma-org.kde.plasma.desktop-appletsrc << 'EOF'
[General]
sysTrayIconsOnly=false
EOF

echo "[ShobikaOs] KDE Plasma customized!"
