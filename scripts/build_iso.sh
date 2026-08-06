#!/usr/bin/env bash
set -ex

echo "=== Step 1: Show system info ==="
uname -a
if [ -f /etc/os-release ]; then
  cat /etc/os-release
fi

echo "=== Step 2: Setup Pacman mirrors ==="
cat > /etc/pacman.d/mirrorlist << 'MIRRORLIST'
Server = https://geo.mirror.pkgbuild.com/$repo/os/$arch
Server = https://mirror.rackspace.com/archlinux/$repo/os/$arch
Server = https://mirrors.kernel.org/archlinux/$repo/os/$arch
Server = https://arch.hu.fo/archlinux/$repo/os/$arch
MIRRORLIST

echo "=== Step 3: Disable Pacman signature verification & Parallel Downloads ==="
sed -i 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf
sed -i 's/^LocalFileSigLevel.*/LocalFileSigLevel = Never/' /etc/pacman.conf
sed -i 's/^ParallelDownloads.*/#ParallelDownloads = 5/' /etc/pacman.conf

if ! grep -q "^SigLevel = Never" /etc/pacman.conf; then
  echo "SigLevel = Never" >> /etc/pacman.conf
fi

echo "=== Step 4: Update pacman database ==="
pacman -Sy --noconfirm --noprogressbar

echo "=== Step 5.1: Install Archiso & Isoburn ==="
pacman -S --noconfirm --needed --noprogressbar archiso mtools libisoburn squashfs-tools

echo "=== Step 5.2: Install Bootloader tools ==="
pacman -S --noconfirm --needed --noprogressbar grub

echo "=== Step 5.3: Install Compiler tools ==="
pacman -S --noconfirm --needed --noprogressbar gcc pkgconf

echo "=== Step 5.4: Install GTK4 UI libraries ==="
pacman -S --noconfirm --needed --noprogressbar gtk4 libadwaita cairo pango gdk-pixbuf2 glib2

echo "=== Step 6: Compile GTK4 Installer ==="
mkdir -p iso/airootfs/usr/bin

gcc -O2 \
  -Wno-deprecated-declarations \
  -Wno-incompatible-pointer-types \
  -Wno-int-conversion \
  $(pkg-config --cflags gtk4) \
  installer/main.c \
  $(pkg-config --libs gtk4) \
  -o iso/airootfs/usr/bin/shobika-installer

cp installer/backend.sh iso/airootfs/usr/bin/shobika-install-backend

chmod 755 iso/airootfs/usr/bin/shobika-installer
chmod 755 iso/airootfs/usr/bin/shobika-install-backend

if [ -f iso/airootfs/usr/bin/shobika-live-setup ]; then
  chmod 755 iso/airootfs/usr/bin/shobika-live-setup
fi

echo "=== Step 7: Create Systemd Symlinks ==="
mkdir -p iso/airootfs/etc/systemd/system/multi-user.target.wants
mkdir -p iso/airootfs/etc/systemd/system/graphical.target.wants

ln -sf /usr/lib/systemd/system/NetworkManager.service \
  iso/airootfs/etc/systemd/system/multi-user.target.wants/NetworkManager.service

ln -sf /usr/lib/systemd/system/lightdm.service \
  iso/airootfs/etc/systemd/system/display-manager.service
ln -sf /usr/lib/systemd/system/lightdm.service \
  iso/airootfs/etc/systemd/system/graphical.target.wants/lightdm.service

ln -sf /usr/lib/systemd/system/graphical.target \
  iso/airootfs/etc/systemd/system/default.target

if [ -f iso/airootfs/etc/systemd/system/shobika-live-setup.service ]; then
  ln -sf /etc/systemd/system/shobika-live-setup.service \
    iso/airootfs/etc/systemd/system/multi-user.target.wants/shobika-live-setup.service
fi

echo "=== Step 8: Run mkarchiso ==="
mkdir -p out
rm -rf /tmp/shobika_work

mkarchiso -v -w /tmp/shobika_work -o out iso/

echo "=== Build Complete ==="
ls -lh out/
