#!/usr/bin/env bash
set -ex

echo "=== Step 1: Show system info ==="
uname -a
if [ -f /etc/os-release ]; then
  cat /etc/os-release
fi

echo "=== Step 2: Setup Fast Pacman HTTP mirrors ==="
cat > /etc/pacman.d/mirrorlist << 'MIRRORLIST'
Server = http://geo.mirror.pkgbuild.com/$repo/os/$arch
Server = http://mirror.rackspace.com/archlinux/$repo/os/$arch
Server = http://mirrors.kernel.org/archlinux/$repo/os/$arch
MIRRORLIST

echo "=== Step 3: Configure Pacman settings ==="
sed -i 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf
sed -i 's/^LocalFileSigLevel.*/LocalFileSigLevel = Never/' /etc/pacman.conf

if ! grep -q "^SigLevel = Never" /etc/pacman.conf; then
  echo "SigLevel = Never" >> /etc/pacman.conf
fi

echo "=== Step 4: Full System Upgrade pacman database ==="
stdbuf -o0 -e0 pacman -Syu --noconfirm --noprogressbar --overwrite "*"

echo "=== Step 5a: Install archiso tools ==="
stdbuf -o0 -e0 pacman -S --noconfirm --needed --noprogressbar --overwrite "*" \
  archiso mtools libisoburn squashfs-tools grub

echo "=== Step 5b: Install build tools ==="
stdbuf -o0 -e0 pacman -S --noconfirm --needed --noprogressbar --overwrite "*" \
  gcc pkgconf glib2

echo "=== Step 5c1: Install Cairo & GdkPixbuf ==="
stdbuf -o0 -e0 pacman -S --noconfirm --needed --noprogressbar --overwrite "*" \
  cairo gdk-pixbuf2

echo "=== Step 5c2: Install Pango ==="
stdbuf -o0 -e0 pacman -S --noconfirm --needed --noprogressbar --overwrite "*" \
  pango

echo "=== Step 5d: Install GTK4 & LibAdwaita ==="
stdbuf -o0 -e0 pacman -S --noconfirm --needed --noprogressbar --overwrite "*" \
  gtk4 libadwaita

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

stdbuf -o0 -e0 mkarchiso -v -w /tmp/shobika_work -o out iso/

echo "=== Build Complete ==="
ls -lh out/
