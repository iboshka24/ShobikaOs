#!/usr/bin/env bash
set -e

echo '=== 1. Configure Pacman GeoDNS CDN Mirrors ==='
echo 'Server = https://geo.mirror.pkgbuild.com/$repo/os/$arch' > /etc/pacman.d/mirrorlist
echo 'Server = https://mirror.rackspace.com/archlinux/$repo/os/$arch' >> /etc/pacman.d/mirrorlist
sed -i 's/SigLevel.*/SigLevel = Never/g' /etc/pacman.conf
sed -i 's/LocalFileSigLevel.*/LocalFileSigLevel = Never/g' /etc/pacman.conf

echo '=== 2. Install Build Dependencies ==='
pacman -Sy --noconfirm
pacman -S --noconfirm --needed gcc make binutils patch archiso grub mtools libisoburn squashfs-tools \
  pkgconf gtk4 libadwaita cairo pango gdk-pixbuf2 glib2 graphene

echo '=== 3. Compile Native GTK4 C Installer Binary ==='
mkdir -p iso/airootfs/usr/bin
gcc -O2 -Wno-deprecated-declarations -Wno-incompatible-pointer-types -Wno-int-conversion \
  installer/main.c $(pkg-config --cflags --libs gtk4) \
  -o iso/airootfs/usr/bin/shobika-installer
cp installer/backend.sh iso/airootfs/usr/bin/shobika-install-backend
chmod 755 iso/airootfs/usr/bin/shobika-installer
chmod 755 iso/airootfs/usr/bin/shobika-install-backend
chmod 755 iso/airootfs/usr/bin/shobika-live-setup || true

echo '=== 4. Configure Systemd Live Desktop Services & Symlinks ==='
mkdir -p iso/airootfs/etc/systemd/system/multi-user.target.wants
mkdir -p iso/airootfs/etc/systemd/system/graphical.target.wants

ln -sf /etc/systemd/system/shobika-live-setup.service iso/airootfs/etc/systemd/system/multi-user.target.wants/shobika-live-setup.service
ln -sf /usr/lib/systemd/system/lightdm.service iso/airootfs/etc/systemd/system/display-manager.service
ln -sf /usr/lib/systemd/system/lightdm.service iso/airootfs/etc/systemd/system/graphical.target.wants/lightdm.service
ln -sf /usr/lib/systemd/system/NetworkManager.service iso/airootfs/etc/systemd/system/multi-user.target.wants/NetworkManager.service
ln -sf /usr/lib/systemd/system/graphical.target iso/airootfs/etc/systemd/system/default.target

echo '=== 5. Execute mkarchiso ==='
mkdir -p out
rm -rf /tmp/shobika_work
mkarchiso -v -w /tmp/shobika_work -o out iso/

echo '=== 6. Build Completed Successfully ==='
ls -la out/
