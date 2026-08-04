#!/usr/bin/env bash
iso_name="shobikaos"
iso_label="SHOBIKAOS_$(date +%Y%m)"
iso_publisher="ShobikaOs <https://github.com/shobikaos>"
iso_application="ShobikaOs"
iso_version="$(date +%Y.%m.%d)"
install_dir="arch"
buildmodes=('iso')
bootloader='grub_syslinux'
airootfs_image_tool_options=('comp' 'xz' 'level' '1')
arch=x86_64
pacman_conf='pacman.conf'