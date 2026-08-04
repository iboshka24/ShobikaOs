#!/bin/bash
# ShobikaOs - Cinnamon customization
set -e

echo "[ShobikaOs] Customizing Cinnamon..."

# Dark theme
gsettings set org.cinnamon.desktop.interface gtk-theme 'Arc-Dark'
gsettings set org.cinnamon.desktop.interface icon-theme 'Papirus-Dark'
gsettings set org.cinnamon.desktop.interface cursor-theme 'Adwaita'
gsettings set org.cinnamon.desktop.interface font-name 'Noto Sans 10'
gsettings set org.cinnamon.desktop.interface document-font-name 'Noto Sans 10'
gsettings set org.cinnamon.desktop.interface monospace-font-name 'JetBrains Mono 10'

# Desktop
gsettings set org.cinnamon.desktop.background picture-uri 'file:///usr/share/backgrounds/shobikaos-wallpaper.png'
gsettings set org.cinnamon.desktop.background picture-uri-dark 'file:///usr/share/backgrounds/shobikaos-wallpaper.png'
gsettings set org.cinnamon.desktop.background picture-options 'zoom'

# Panel
gsettings set org.cinnamon panels-enabled "['1:2:bottom']"
gsettings set org.cinnamon panel heights '{1: 32}'

# Terminal colors
mkdir -p ~/.config/cinnamon-terminal
cat > ~/.config/cinnamon-terminal/colorscheme/shobikaos.theme << 'EOF'
[[ShobikaOs]]
bold-color=#5eacd3
foreground-color=#d8dee9
palette=#3b4252:#bf616a:#a3be8c:#ebcb8b:#81a1c1:#b48ead:#88c0d0:#e5e9f0
background-color=#2e3440
bold-color-same-as-fg=false
foreground-color=#d8dee9
background-color=#2e3440
EOF

echo "[ShobikaOs] Cinnamon customized!"
