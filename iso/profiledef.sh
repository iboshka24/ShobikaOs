#!/usr/bin/env bash
# shellcheck disable=SC2034

iso_name="shobikaos"
iso_label="SHOBIKAOS_$(date --date="@${SOURCE_DATE_EPOCH:-$(date +%s)}" +%Y%m)"
iso_publisher="ShobikaOs <https://github.com/iboshka24/ShobikaOs>"
iso_application="ShobikaOs Live/Install DVD"
iso_version="$(date --date="@${SOURCE_DATE_EPOCH:-$(date +%s)}" +%Y.%m.%d)"
install_dir="arch"
buildmodes=('iso')
bootmodes=('bios.syslinux'
           'uefi.grub')
arch="x86_64"
pacman_conf="pacman.conf"
airootfs_image_type="squashfs"
airootfs_image_tool_options=('-comp' 'xz' '-Xbcj' 'x86' '-b' '1M' '-Xdict-size' '1M')
file_permissions=(
  ["/etc/shadow"]="0:0:400"
  ["/root"]="0:0:750"
  ["/usr/bin/shobika-live-setup"]="0:0:755"
)