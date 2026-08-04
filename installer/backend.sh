#!/bin/bash
# ShobikaOs Installation Backend Script
# Reads /tmp/shobika-installer.conf and executes full Arch installation

set -e

CONF_FILE="/tmp/shobika-installer.conf"
if [ -f "$CONF_FILE" ]; then
    source "$CONF_FILE"
else
    echo "Configuration file not found!"
    exit 1
fi

DISK="${DISK:-/dev/sda}"
USERNAME="${USERNAME:-user}"
FULLNAME="${FULLNAME:-Shobika User}"
PASSWORD="${PASSWORD:-shobikaos}"
HOSTNAME="${HOSTNAME:-shobika-pc}"
DESKTOP="${DESKTOP:-cinnamon}"
DISPLAY_MANAGER="${DISPLAY_MANAGER:-auto}"
WALLPAPER="${WALLPAPER:-shobikaos-aurora.png}"

echo "Starting installation on $DISK..."

# 1. Partition disk (UEFI / GPT)
parted -s "$DISK" mklabel gpt
parted -s "$DISK" mkpart ESP fat32 1MiB 512MiB
parted -s "$DISK" set 1 esp on
parted -s "$DISK" mkpart primary ext4 512MiB 100%

if [[ "$DISK" == *"nvme"* ]] || [[ "$DISK" == *"mmcblk"* ]]; then
    BOOT_PART="${DISK}p1"
    ROOT_PART="${DISK}p2"
else
    BOOT_PART="${DISK}1"
    ROOT_PART="${DISK}2"
fi

mkfs.fat -F32 "$BOOT_PART"
mkfs.ext4 -F "$ROOT_PART"

mount "$ROOT_PART" /mnt
mkdir -p /mnt/boot
mount "$BOOT_PART" /mnt/boot

# 2. Pacstrap base Arch system
pacstrap /mnt base linux linux-firmware base-devel git sudo nano vim networkmanager pipewire pipewire-pulse pipewire-alsa wireplumber grub efibootmgr fastfetch papirus-icon-theme

# 3. Generate fstab
genfstab -U /mnt >> /mnt/etc/fstab

# 4. Configure System via arch-chroot
arch-chroot /mnt bash -c "
ln -sf /usr/share/zoneinfo/UTC /etc/localtime
hwclock --systohc
echo '$HOSTNAME' > /etc/hostname

echo 'en_US.UTF-8 UTF-8' >> /etc/locale.gen
echo 'ru_RU.UTF-8 UTF-8' >> /etc/locale.gen
locale-gen
echo 'LANG=ru_RU.UTF-8' > /etc/locale.conf

# User creation
useradd -m -g users -G wheel,video,audio,storage,network,input -s /bin/bash '$USERNAME'
echo '$USERNAME:$PASSWORD' | chpasswd
echo 'root:$PASSWORD' | chpasswd
echo '%wheel ALL=(ALL:ALL) ALL' > /etc/sudoers.d/wheel

# Enable services
systemctl enable NetworkManager

# Install Desktop Environment
case '$DESKTOP' in
    cinnamon)
        pacman -S --noconfirm cinnamon nemo gnome-terminal lightdm lightdm-gtk-greeter
        systemctl enable lightdm
        ;;
    gnome)
        pacman -S --noconfirm gnome gnome-extra gdm
        systemctl enable gdm
        ;;
    kde)
        pacman -S --noconfirm plasma kde-applications sddm
        systemctl enable sddm
        ;;
    xfce)
        pacman -S --noconfirm xfce4 xfce4-goodies lightdm lightdm-gtk-greeter
        systemctl enable lightdm
        ;;
    hyprland)
        pacman -S --noconfirm hyprland waybar wofi foot sddm
        systemctl enable sddm
        ;;
esac

# Override DM if specified
if [ '$DISPLAY_MANAGER' != 'auto' ]; then
    systemctl disable lightdm gdm sddm || true
    pacman -S --noconfirm '$DISPLAY_MANAGER' || true
    systemctl enable '$DISPLAY_MANAGER'
fi

# Install Bootloader
grub-install --target=x86_64-efi --efi-directory=/boot --bootloader-id=ShobikaOs
grub-mkconfig -o /boot/grub/grub.cfg

echo 'Installation complete!'
"
