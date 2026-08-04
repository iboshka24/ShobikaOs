#!/bin/bash
# ShobikaOs GRUB theme installer
# Run as root after installation

set -e

THEME_DIR="/boot/grub/themes/shobikaos"
WALLPAPER="/usr/share/backgrounds/shobikaos-wallpaper.png"

echo "[ShobikaOs] Installing GRUB theme..."

mkdir -p "$THEME_DIR"

# Copy theme file
cat > "$THEME_DIR/theme.txt" << 'THEME_EOF'
# ShobikaOs GRUB Theme
title-color: "#cdd6f4"
title-font: "JetBrains Mono Bold 32"
message-color: "#a6adc8"
message-font: "Noto Sans Regular 16"
terminal-border: "#313244"
terminal-box: "#181825@100"
terminal-font: "JetBrains Mono Regular 14"

+ boot_menu {
  left = 10%
  top = 20%
  width = 80%
  height = 60%
  item_font = "Noto Sans Regular 18"
  item_color = "#cdd6f4"
  selected_item_color = "#1e1e2e"
  selected_item_font = "Noto Sans Bold 18"
  item_height = 40
  item_padding = 16
  item_spacing = 8
  scrollbar = true
  scrollbar_thumb = "#89b4fa"
  scrollbar_track = "#313244"
}

+ progress_bar {
  id = "__timeout__"
  left = 10%
  top = 85%
  width = 80%
  height = 8
  bg_color = "#313244"
  fg_color = "#89b4fa"
  border_color = "#45475a"
  border_width = 1
  text = "@TIMEOUT_NOTIFICATION_LONG@"
  text_color = "#a6adc8"
  font = "Noto Sans Regular 14"
  text_align = "center"
}

+ image {
  left = 0
  top = 0
  width = 100%
  height = 100%
  file = "background.png"
}

+ label {
  left = 10%
  top = 5%
  width = 80%
  height = 60
  text = "ShobikaOs"
  color = "#89b4fa"
  font = "JetBrains Mono Bold 48"
  align = "center"
}

+ label {
  left = 10%
  top = 12%
  width = 80%
  height = 40
  text = "Arch-based Linux Distribution"
  color = "#a6adc8"
  font = "Noto Sans Regular 18"
  align = "center"
}

+ label {
  left = 10%
  top = 90%
  width = 80%
  height = 30
  text = "Use ↑/↓ to select, Enter to boot, 'e' to edit, 'c' for command line"
  color = "#6c7086"
  font = "Noto Sans Regular 13"
  align = "center"
}
THEME_EOF

# Convert SVG to PNG for GRUB background
if command -v magick &> /dev/null; then
    magick "/usr/share/backgrounds/shobikaos-wallpaper.svg" -resize 1920x1080 "$THEME_DIR/background.png"
elif command -v convert &> /dev/null; then
    convert "/usr/share/backgrounds/shobikaos-wallpaper.svg" -resize 1920x1080 "$THEME_DIR/background.png"
elif command -v rsvg-convert &> /dev/null; then
    rsvg-convert -w 1920 -h 1080 "/usr/share/backgrounds/shobikaos-wallpaper.svg" -o "$THEME_DIR/background.png"
else
    echo "[ShobikaOs] Warning: No image converter found, using default background"
    # Create a simple fallback
    cat > "$THEME_DIR/background.png" << 'EOF'
iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==
EOF
    base64 -d "$THEME_DIR/background.png" > "$THEME_DIR/background.png.tmp" && mv "$THEME_DIR/background.png.tmp" "$THEME_DIR/background.png"
fi

# Update GRUB config
if grep -q "GRUB_THEME" /etc/default/grub; then
    sed -i "s|GRUB_THEME=.*|GRUB_THEME=\"$THEME_DIR/theme.txt\"|" /etc/default/grub
else
    echo "GRUB_THEME=\"$THEME_DIR/theme.txt\"" >> /etc/default/grub
fi

# Update GRUB
grub-mkconfig -o /boot/grub/grub.cfg

echo "[ShobikaOs] GRUB theme installed successfully!"