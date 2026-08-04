#!/bin/bash
# ShobikaOs - XFCE customization
set -e

echo "[ShobikaOs] Customizing XFCE..."

# Theme
xfconf-query -c xsettings -p /Net/ThemeName -s "Arc-Dark"
xfconf-query -c xsettings -p /Net/IconThemeName -s "Papirus-Dark"
xfconf-query -c xsettings -p /Gtk/FontName -s "Noto Sans 10"
xfconf-query -c xsettings -p /Gtk/MonospaceFontName -s "JetBrains Mono 10"

# Wallpaper
xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitor0/workspace0/last-image -s "/usr/share/backgrounds/shobikaos-wallpaper.png"

# Panel
xfconf-query -c xfce4-panel -p /panels/panel-1/size -s 32
xfconf-query -c xfce4-panel -p /panels/panel-1/background-style -s 1

# Terminal colors
mkdir -p ~/.config/xfce4/terminal
cat > ~/.config/xfce4/terminal/terminalrc << 'EOF'
[Configuration]
FontName=JetBrains Mono 10
ColorForeground=#d8dee9
ColorBackground=#2e3440
ColorPalette=#3b4252:#bf616a:#a3be8c:#ebcb8b:#81a1c1:#b48ead:#88c0d0:#e5e9f0
EOF

echo "[ShobikaOs] XFCE customized!"
