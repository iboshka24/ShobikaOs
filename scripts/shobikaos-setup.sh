#!/bin/bash
# ShobikaOs Post-Install Setup Script
# Run after first boot to configure additional settings

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     ShobikaOs Post-Install Setup       ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo

# Check root
if [[ $EUID -eq 0 ]]; then
    echo -e "${RED}Не запускайте от root. Используйте обычного пользователя.${NC}"
    exit 1
fi

# Function to ask yes/no
ask() {
    local prompt="$1"
    local default="$2"
    read -p "$(echo -e ${CYAN}"$prompt [Y/n]: "${NC})" answer
    answer=${answer:-$default}
    [[ "$answer" =~ ^[Yy]$ ]]
}

# 1. Update system
echo -e "${GREEN}[1/7]${NC} Обновление системы..."
sudo pacman -Syu --noconfirm

# 2. Install AUR helper
if ! command -v yay &> /dev/null && ! command -v paru &> /dev/null; then
    if ask "Установить AUR хелпер (yay)?" "Y"; then
        echo -e "${GREEN}[2/7]${NC} Установка yay..."
        cd /tmp
        git clone https://aur.archlinux.org/yay-bin.git
        cd yay-bin
        makepkg -si --noconfirm
        cd ~
        rm -rf /tmp/yay-bin
        echo -e "${GREEN}yay установлен!${NC}"
    fi
else
    echo -e "${GREEN}[2/7]${NC} AUR хелпер уже установлен."
fi

# 3. Flatpak setup
if command -v flatpak &> /dev/null; then
    echo -e "${GREEN}[3/7]${NC} Настройка Flatpak..."
    flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
    echo -e "${GREEN}Flathub добавлен!${NC}"
fi

# 4. XDG directories
echo -e "${GREEN}[4/7]${NC} Создание XDG директорий..."
xdg-user-dirs-update 2>/dev/null || true
mkdir -p ~/Pictures/Screenshots ~/Videos ~/Projects ~/Downloads

# 5. Optimize mirrors
if ask "Оптимизировать зеркала pacman (reflector)?" "Y"; then
    echo -e "${GREEN}[5/7]${NC} Оптимизация зеркал..."
    sudo reflector --latest 10 --sort rate --save /etc/pacman.d/mirrorlist
    echo -e "${GREEN}Зеркала обновлены!${NC}"
fi

# 6. Install additional apps
if ask "Установить рекомендуемые приложения? (VS Code, Spotify, Discord)" "n"; then
    echo -e "${GREEN}[6/7]${NC} Установка приложений..."
    if command -v flatpak &> /dev/null; then
        flatpak install -y flathub com.spotify.Client
        flatpak install -y flathub com.discordapp.Discord
        flatpak install -y flathub com.visualstudio.code
    fi
fi

# 7. Enable TRIM timer
echo -e "${GREEN}[7/7]${NC} Финальные настройки..."
sudo systemctl enable --now fstrim.timer 2>/dev/null || true

echo
echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║   ShobikaOs настроен! 🎉              ║${NC}"
echo -e "${GREEN}║   Перезагрузите для применения.         ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
