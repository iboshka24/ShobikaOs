#!/usr/bin/env bash
set -ex

echo "=== Step 1: Show system info ==="
uname -a
cat /etc/os-release 2>/dev/null || true

echo "=== Step 2: Setup Pacman mirrors ==="
cat > /etc/pacman.d/mirrorlist << 'MIRRORLIST'
Server = https://geo.mirror.pkgbuild.com/$repo/os/$arch
Server = https://mirror.rackspace.com/archlinux/$repo/os/$arch
MIRRORLIST

echo "=== Step 3: Disable Pacman signature verification ==="
sed -i 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf
sed -i 's/^LocalFileSigLevel.*/LocalFileSigLevel = Never/' /etc/pacman.conf

# Ensure SigLevel is set in each repo section if not present globally
grep -q "^SigLevel" /etc/pacman.conf || echo "SigLevel = Never" >> /etc/pacman.conf

echo "=== Step 4: Update pacman database ==="
pacman -Sy --noconfirm

echo "=== Step 5: Install build tools ==="
pacman -S --noconfirm --needed \
  archiso \
  grub \
  mtools \
  libisoburn \
  squashfs-tools \
  gcc \
  pkgconf \
  gtk4 \
  libadwaita \
  cairo \
  pango \
  gdk-pixbuf2 \
  glib2

echo "=== Step 6: Compile GTK4 Installer ==="
mkdir -p iso/airootfs/usr/bin

# Build the installer binary
gcc -O2 \
  -Wno-deprecated-declarations \
  -Wno-incompatible-pointer-types \
  -Wno-int-conversion \
  $(pkg-config --cflags gtk4) \
  installer/main.c \
  $(pkg-config --libs gtk4) \
  -o iso/airootfs/usr/bin/shobika-installer

# Copy backend script
cp installer/backend.sh iso/airootfs/usr/bin/shobika-install-backend

# Set permissions
chmod 755 iso/airootfs/usr/bin/shobika-installer
chmod 755 iso/airootfs/usr/bin/shobika-install-backend

# Fix shobika-live-setup permissions if it exists
[ -f iso/airootfs/usr/bin/shobika-live-setup ] && chmod 755 iso/airootfs/usr/bin/shobika-live-setup

echo "=== Step 7: Create Systemd Symlinks ==="
mkdir -p iso/airootfs/etc/systemd/system/multi-user.target.wants
mkdir -p iso/airootfs/etc/systemd/system/graphical.target.wants

# Enable NetworkManager
ln -sf /usr/lib/systemd/system/NetworkManager.service \
  iso/airootfs/etc/systemd/system/multi-user.target.wants/NetworkManager.service

# Enable LightDM
ln -sf /usr/lib/systemd/system/lightdm.service \
  iso/airootfs/etc/systemd/system/display-manager.service
ln -sf /usr/lib/systemd/system/lightdm.service \
  iso/airootfs/etc/systemd/system/graphical.target.wants/lightdm.service

# Set graphical default target
ln -sf /usr/lib/systemd/system/graphical.target \
  iso/airootfs/etc/systemd/system/default.target

# Enable live setup service if it exists
[ -f iso/airootfs/etc/systemd/system/shobika-live-setup.service ] && \
  ln -sf /etc/systemd/system/shobika-live-setup.service \
    iso/airootfs/etc/systemd/system/multi-user.target.wants/shobika-live-setup.service

echo "=== Step 8: Run mkarchiso ==="
mkdir -p out
rm -rf /tmp/shobika_work

mkarchiso -v -w /tmp/shobika_work -o out iso/

echo "=== Build Complete ==="
ls -lh out/
