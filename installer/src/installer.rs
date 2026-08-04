use anyhow::Result;
use serde::{Deserialize, Serialize};
use std::sync::{Arc, Mutex};
use std::fs;
use crate::hardware::HardwareInfo;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InstallConfig {
    pub language: String,
    pub keyboard_layout: String,
    pub keyboard_variant: String,
    pub disk: String,
    pub use_btrfs: bool,
    pub use_luks: bool,
    pub username: String,
    pub fullname: String,
    pub password: String,
    pub desktop: String,
    pub display_manager: String,
    pub wallpaper: String,
    pub install_gaming: bool,
    pub install_multimedia: bool,
    pub install_office: bool,
    pub install_browser: bool,
    pub install_flatpak: bool,
    pub install_bluetooth: bool,
    pub install_printing: bool,
}

impl Default for InstallConfig {
    fn default() -> Self {
        Self {
            language: "en_US.UTF-8".into(),
            keyboard_layout: "us".into(),
            keyboard_variant: "".into(),
            disk: String::new(),
            use_btrfs: true,
            use_luks: false,
            username: String::new(),
            fullname: String::new(),
            password: String::new(),
            hostname: "shobikaos".into(),
            desktop: "cinnamon".into(),
            display_manager: "auto".into(),
            wallpaper: "shobikaos-aurora.png".into(),
            install_gaming: true,
            install_multimedia: true,
            install_office: false,
            install_browser: true,
            install_flatpak: true,
            install_bluetooth: true,
            install_printing: false,
        }
    }
}

pub struct Installer {
    pub config: InstallConfig,
    mount_point: String,
}

impl Installer {
    pub fn new() -> Self {
        Self {
            config: InstallConfig::default(),
            mount_point: "/mnt".into(),
        }
    }

    pub fn partition_disk(&mut self) -> Result<()> {
        let disk = &self.config.disk;
        
        Command::new("wipefs").args(["-a", disk]).status()?;
        
        if self.config.use_luks {
            // LUKS2 encryption
            Command::new("cryptsetup").args([
                "luksFormat", "--type", "luks2", "--batch-mode", disk
            ]).status()?;
            
            Command::new("cryptsetup").args(["open", disk, "cryptroot"]).status()?;
            
            let mapper = "/dev/mapper/cryptroot";
            
            Command::new("parted").args([mapper, "mklabel", "gpt"]).status()?;
            Command::new("parted").args([mapper, "mkpart", "ESP", "fat32", "1MiB", "513MiB"]).status()?;
            Command::new("parted").args([mapper, "set", "1", "esp", "on"]).status()?;
            Command::new("parted").args([mapper, "mkpart", "root", "ext4", "513MiB", "100%"]).status()?;
            
            Command::new("mkfs.fat").args(["-F32", &format!("{}p1", mapper)]).status()?;
            
            if self.config.use_btrfs {
                Command::new("mkfs.btrfs").args(["-f", "-L", "ROOT", &format!("{}p2", mapper)]).status()?;
            } else {
                Command::new("mkfs.ext4").args(["-F", "-L", "ROOT", &format!("{}p2", mapper)]).status()?;
            }
            
            fs::create_dir_all(&self.mount_point)?;
            Command::new("mount").args([&format!("{}p2", mapper), &self.mount_point]).status()?;
            fs::create_dir_all(&format!("{}/boot", self.mount_point))?;
            Command::new("mount").args([&format!("{}p1", mapper), &format!("{}/boot", self.mount_point)]).status()?;
            
            if self.config.use_btrfs {
                self.create_btrfs_subvolumes()?;
            }
        } else {
            // No encryption
            Command::new("parted").args([disk, "mklabel", "gpt"]).status()?;
            Command::new("parted").args([disk, "mkpart", "ESP", "fat32", "1MiB", "513MiB"]).status()?;
            Command::new("parted").args([disk, "set", "1", "esp", "on"]).status()?;
            Command::new("parted").args([disk, "mkpart", "root", "ext4", "513MiB", "100%"]).status()?;
            
            Command::new("mkfs.fat").args(["-F32", &format!("{}1", disk)]).status()?;
            
            if self.config.use_btrfs {
                Command::new("mkfs.btrfs").args(["-f", "-L", "ROOT", &format!("{}2", disk)]).status()?;
            } else {
                Command::new("mkfs.ext4").args(["-F", "-L", "ROOT", &format!("{}2", disk)]).status()?;
            }
            
            fs::create_dir_all(&self.mount_point)?;
            Command::new("mount").args([&format!("{}2", disk), &self.mount_point]).status()?;
            fs::create_dir_all(&format!("{}/boot", self.mount_point))?;
            Command::new("mount").args([&format!("{}1", disk), &format!("{}/boot", self.mount_point)]).status()?;
            
            if self.config.use_btrfs {
                self.create_btrfs_subvolumes()?;
            }
        }
        
        Ok(())
    }

    fn create_btrfs_subvolumes(&self) -> Result<()> {
        let subvols = ["@", "@home", "@var_log", "@pkg", "@snapshots"];
        for sv in subvols {
            Command::new("btrfs").args(["subvolume", "create", &format!("{}/{}", self.mount_point, sv)]).status()?;
        }
        
        Command::new("umount").args([&self.mount_point]).status()?;
        
        let root_part = if self.config.use_luks {
            "/dev/mapper/cryptroot"
        } else {
            &format!("{}2", self.config.disk)
        };
        
        Command::new("mount").args(["-o", "noatime,compress=zstd,subvol=@", root_part, &self.mount_point]).status()?;
        
        for sv in &["home", "var_log", "pkg", "snapshots"] {
            let path = format!("{}/{}", self.mount_point, sv.trim_start_matches('@'));
            fs::create_dir_all(&path)?;
            Command::new("mount").args(["-o", &format!("noatime,compress=zstd,subvol=@{}", sv), root_part, &path]).status()?;
        }
        
        fs::create_dir_all(&format!("{}/boot", self.mount_point))?;
        let boot_part = if self.config.use_luks { "/dev/mapper/cryptroot" } else { &format!("{}1", self.config.disk) };
        Command::new("mount").args([boot_part, &format!("{}/boot", self.mount_point)]).status()?;
        
        Ok(())
    }

    pub fn install_base(&self) -> Result<()> {
        let packages = [
            "base", "linux", "linux-firmware", "linux-headers", "base-devel",
            "networkmanager", "sudo", "vim", "nano", "git",
            "btrfs-progs", "dosfstools", "e2fsprogs", "ntfs-3g",
            "grub", "efibootmgr", "os-prober",
            "pciutils", "usbutils", "dmidecode", "lshw",
            "man-db", "man-pages", "texinfo",
            "bash-completion", "wget", "curl", "rsync",
            "unzip", "p7zip", "unrar",
            "htop", "btop", "fastfetch",
            "fzf", "ripgrep", "fd", "bat", "eza", "zoxide",
            "pacman-contrib",
            "xdg-user-dirs", "xdg-utils",
            "shobikaos-branding",
        ];
        
        let mut args = vec!["pacstrap", "-K", &self.mount_point];
        args.extend(packages);
        Command::new("pacstrap").args(&args).status()?;
        
        let fstab_output = Command::new("genfstab").args(["-U", &self.mount_point]).output()?;
        fs::write(&format!("{}/etc/fstab", self.mount_point), &fstab_output.stdout)?;
        
        Ok(())
    }

    pub fn install_desktop(&self) -> Result<()> {
        let packages = self.desktop_packages();
        let mut args = vec!["pacstrap", &self.mount_point];
        args.extend(packages);
        Command::new("pacstrap").args(&args).status()?;
        Ok(())
    }

    pub fn get_effective_dm(&self) -> String {
        if self.config.display_manager != "auto" && !self.config.display_manager.is_empty() {
            self.config.display_manager.clone()
        } else {
            match self.config.desktop.as_str() {
                "gnome" => "gdm".to_string(),
                "kde" | "hyprland" => "sddm".to_string(),
                _ => "lightdm".to_string(),
            }
        }
    }

    fn desktop_packages(&self) -> Vec<&'static str> {
        let mut pkgs = match self.config.desktop.as_str() {
            "cinnamon" => vec!["cinnamon", "cinnamon-terminal", "nemo", "file-roller", "eog", "evince", "gnome-calculator"],
            "gnome" => vec!["gnome", "gnome-extra", "gnome-tweaks"],
            "kde" => vec!["plasma", "kde-applications", "kvantum", "konsole", "dolphin"],
            "xfce" => vec!["xfce4", "xfce4-goodies", "thunar"],
            "hyprland" => vec!["hyprland", "waybar", "wofi", "foot", "hyprpaper", "hyprlock", "hypridle", "xdg-desktop-portal-hyprland", "polkit-gnome", "dunst", "cliphist", "wl-clipboard", "grim", "slurp", "brightnessctl", "pavucontrol", "network-manager-applet"],
            _ => vec!["cinnamon"],
        };

        let dm = self.get_effective_dm();
        match dm.as_str() {
            "sddm" => pkgs.extend_from_slice(&["sddm", "qt5-graphicaleffects", "qt5-quickcontrols2", "qt5-svg"]),
            "gdm" => pkgs.push("gdm"),
            "lightdm" => pkgs.extend_from_slice(&["lightdm", "lightdm-gtk-greeter", "lightdm-webkit2-greeter"]),
            "ly" => pkgs.push("ly"),
            _ => {}
        }

        pkgs
    }

    pub fn configure_system(&self) -> Result<()> {
        // hostname
        fs::write(&format!("{}/etc/hostname", self.mount_point), &self.config.hostname)?;
        
        // locale
        let locale_gen = format!("{} UTF-8\n", self.config.language.replace(".UTF-8", ""));
        fs::write(&format!("{}/etc/locale.gen", self.mount_point), locale_gen)?;
        
        let locale_conf = format!("LANG={}\n", self.config.language);
        fs::write(&format!("{}/etc/locale.conf", self.mount_point), locale_conf)?;
        self.chroot("locale-gen")?;
        
        // keyboard
        let vconsole = format!("KEYMAP={}\n", self.config.keyboard_layout);
        fs::write(&format!("{}/etc/vconsole.conf", self.mount_point), vconsole)?;
        
        // hosts
        let hosts = format!("127.0.0.1\tlocalhost\n::1\t\tlocalhost\n127.0.1.1\t{}.localdomain\t{}\n", self.config.hostname, self.config.hostname);
        fs::write(&format!("{}/etc/hosts", self.mount_point), hosts)?;
        
        // user
        self.chroot(&format!("useradd -m -G wheel -s /bin/bash -c \"{}\" {}", self.config.fullname, self.config.username))?;
        self.chroot(&format!("echo '{}:{}' | chpasswd", self.config.username, self.config.password))?;
        self.chroot(&format!("echo 'root:{}' | chpasswd", self.config.password))?;
        
        // sudo
        fs::write(&format!("{}/etc/sudoers.d/10-wheel", self.mount_point), "%wheel ALL=(ALL:ALL) ALL\n")?;
        
        // services
        self.chroot("systemctl enable NetworkManager")?;
        self.chroot("systemctl enable systemd-resolved")?;
        self.chroot("systemctl enable systemd-timesyncd")?;
        self.chroot("systemctl enable fstrim.timer")?;
        self.chroot("systemctl enable paccache.timer")?;
        
        let dm = self.get_effective_dm();
        self.chroot(&format!("systemctl enable {}", dm))?;
        
        // Plymouth for graphical boot
        self.chroot("systemctl enable plymouth-start")?;
        
        // mkinitcpio
        let hooks = if self.config.use_luks {
            "HOOKS=(base udev autodetect microcode modconf kms keyboard keymap consolefont block encrypt filesystems fsck plymouth)"
        } else {
            "HOOKS=(base udev autodetect microcode modconf kms keyboard keymap consolefont block filesystems fsck plymouth)"
        };
        let mut mkinitcpio = fs::read_to_string(&format!("{}/etc/mkinitcpio.conf", self.mount_point))?;
        mkinitcpio = mkinitcpio.lines()
            .map(|l| if l.starts_with("HOOKS=") { hooks } else { l })
            .collect::<Vec<_>>()
            .join("\n");
        fs::write(&format!("{}/etc/mkinitcpio.conf", self.mount_point), mkinitcpio)?;
        self.chroot("mkinitcpio -P")?;
        
        // GRUB with theme
        self.chroot("grub-install --target=x86_64-efi --efi-directory=/boot --bootloader-id=ShobikaOs")?;
        self.chroot("grub-mkconfig -o /boot/grub/grub.cfg")?;
        
        // Apply GRUB theme
        self.chroot("/usr/bin/shobikaos-install-grub-theme")?;
        
        // Configure display manager theme
        self.configure_display_manager()?;
        
        // Apply branding to user configs
        self.apply_branding()?;
        
        Ok(())
    }

    fn configure_display_manager(&self) -> Result<()> {
        let dm = self.get_effective_dm();
        
        match dm.as_str() {
            "sddm" => {
                // SDDM theme
                fs::write(&format!("{}/etc/sddm.conf", self.mount_point), 
                    "[Theme]\nCurrent=shobikaos\n")?;
            }
            "lightdm" => {
                // LightDM webkit2 greeter
                fs::write(&format!("{}/etc/lightdm/lightdm-webkit2-greeter.conf", self.mount_point),
                    "[greeter]\nwebkit_theme = shobikaos\n")?;
                // Also update lightdm.conf to use webkit2 greeter
                let mut lightdm_conf = fs::read_to_string(&format!("{}/etc/lightdm/lightdm.conf", self.mount_point))?;
                lightdm_conf = lightdm_conf.replace(
                    "greeter-session=lightdm-gtk-greeter",
                    "greeter-session=lightdm-webkit2-greeter"
                );
                fs::write(&format!("{}/etc/lightdm/lightdm.conf", self.mount_point), lightdm_conf)?;
            }
            "gdm" => {
                // GDM theme via gsettings (applied on first login)
            }
            "ly" => {
                // Ly TUI display manager
            }
            _ => {}
        }
        
        Ok(())
    }

    fn apply_branding(&self) -> Result<()> {
        // The branding package already provides /etc/skel configs
        // We just need to ensure they're copied for the created user
        
        // Copy skel to user home (already done by useradd -m)
        // But ensure the configs are in place
        
        // Set GTK theme globally
        let gtk3_settings = r#"[Settings]
gtk-theme-name=ShobikaOs-Dark
gtk-icon-theme-name=Papirus-Dark
gtk-font-name=Noto Sans 10
gtk-cursor-theme-name=Adwaita
gtk-cursor-theme-size=24
gtk-toolbar-style=GTK_TOOLBAR_BOTH
gtk-toolbar-icon-size=GTK_ICON_SIZE_LARGE_TOOLBAR
gtk-button-images=1
gtk-menu-images=1
gtk-enable-event-sounds=1
gtk-enable-input-feedback-sounds=1
gtk-xft-antialias=1
gtk-xft-hinting=1
gtk-xft-hintstyle=hintslight
gtk-xft-rgba=rgb
gtk-application-prefer-dark-theme=1
gtk-decoration-layout=icon:minimize,maximize,close
"#;
        fs::write(&format!("{}/etc/gtk-3.0/settings.ini", self.mount_point), gtk3_settings)?;
        
        let gtk4_settings = r#"[Settings]
gtk-theme-name=ShobikaOs-Dark
gtk-icon-theme-name=Papirus-Dark
gtk-font-name=Noto Sans 10
gtk-cursor-theme-name=Adwaita
gtk-cursor-theme-size=24
gtk-application-prefer-dark-theme=1
gtk-decoration-layout=icon:minimize,maximize,close
"#;
        fs::write(&format!("{}/etc/gtk-4.0/settings.ini", self.mount_point), gtk4_settings)?;
        
        // Environment variables for all users
        let env_file = r#"# ShobikaOs Environment
export GTK_THEME=ShobikaOs-Dark
export QT_QPA_PLATFORMTHEME=gtk2
export QT_STYLE_OVERRIDE=gtk2
export DISTRO_NAME=ShobikaOs
export DISTRO_VERSION=rolling
export DISTRO_URL=https://github.com/shobikaos/ShobikaOs
"#;
        fs::write(&format!("{}/etc/profile.d/shobikaos.sh", self.mount_point), env_file)?;
        
        // Plymouth theme
        fs::write(&format!("{}/etc/plymouth/plymouthd.conf", self.mount_point),
            "[Daemon]\nTheme=shobikaos\nShowDelay=0\n")?;
        
        Ok(())
    }

    pub fn install_bootloader(&self) -> Result<()> {
        // Already done in configure_system
        Ok(())
    }

    pub fn finalize(&self) -> Result<()> {
        self.chroot("pacman -Sc --noconfirm")?;
        Command::new("sync").status()?;
        Ok(())
    }

    pub fn install_hardware_drivers(&self) -> Result<()> {
        let hw = HardwareInfo::detect();
        
        // GPU drivers
        let gpu_pkgs = hw.gpu_packages();
        if !gpu_pkgs.is_empty() {
            let mut args: Vec<&str> = vec!["-K", &self.mount_point];
            args.extend(&gpu_pkgs);
            Command::new("pacstrap").args(&args).status()?;
        }
        
        // CPU microcode
        let cpu_pkgs = hw.cpu_packages();
        if !cpu_pkgs.is_empty() {
            let mut args: Vec<&str> = vec![&self.mount_point];
            args.extend(&cpu_pkgs);
            Command::new("pacstrap").args(&args).status()?;
        }
        
        // Extra hardware packages
        let extra_pkgs = hw.extra_packages();
        if !extra_pkgs.is_empty() {
            let mut args: Vec<&str> = vec![&self.mount_point];
            args.extend(&extra_pkgs);
            Command::new("pacstrap").args(&args).status()?;
        }
        
        // Enable services
        if hw.has_bluetooth {
            self.chroot("systemctl enable bluetooth")?;
        }
        if hw.is_laptop {
            self.chroot("systemctl enable tlp")?;
            self.chroot("systemctl enable acpid")?;
        }
        
        Ok(())
    }

    pub fn install_multimedia_stack(&self) -> Result<()> {
        if !self.config.install_multimedia {
            return Ok(());
        }
        
        let packages = [
            // PipeWire audio stack
            "pipewire", "pipewire-pulse", "pipewire-alsa", "pipewire-jack",
            "wireplumber", "lib32-pipewire", "lib32-pipewire-jack",
            // Codecs
            "ffmpeg", "gstreamer", "gst-plugins-base", "gst-plugins-good",
            "gst-plugins-bad", "gst-plugins-ugly", "gst-libav",
            // Video acceleration
            "libva", "libva-utils", "vdpauinfo",
        ];
        
        let mut args: Vec<&str> = vec![&self.mount_point];
        args.extend(&packages);
        Command::new("pacstrap").args(&args).status()?;
        
        // Enable PipeWire user services
        self.chroot(&format!(
            "su - {} -c 'systemctl --user enable pipewire pipewire-pulse wireplumber'",
            self.config.username
        ))?;
        
        Ok(())
    }

    pub fn install_gaming_stack(&self) -> Result<()> {
        if !self.config.install_gaming {
            return Ok(());
        }
        
        let packages = [
            "steam", "lutris", "wine-staging", "winetricks",
            "gamemode", "lib32-gamemode",
            "mangohud", "lib32-mangohud",
            "vulkan-tools", "vkd3d", "lib32-vkd3d",
        ];
        
        let mut args: Vec<&str> = vec![&self.mount_point];
        args.extend(&packages);
        Command::new("pacstrap").args(&args).status()?;
        
        Ok(())
    }

    pub fn install_optional_packages(&self) -> Result<()> {
        let mut packages: Vec<&str> = Vec::new();
        
        if self.config.install_browser {
            packages.push("firefox");
        }
        if self.config.install_office {
            packages.extend_from_slice(&["libreoffice-fresh", "hunspell", "hunspell-en_us"]);
        }
        if self.config.install_flatpak {
            packages.push("flatpak");
        }
        if self.config.install_printing {
            packages.extend_from_slice(&["cups", "cups-pdf", "system-config-printer", "gutenprint"]);
        }
        
        if !packages.is_empty() {
            let mut args: Vec<&str> = vec![&self.mount_point];
            args.extend(&packages);
            Command::new("pacstrap").args(&args).status()?;
        }
        
        // Enable services for optional packages
        if self.config.install_flatpak {
            self.chroot("flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo")?;
        }
        if self.config.install_printing {
            self.chroot("systemctl enable cups")?;
        }
        
        Ok(())
    }

    pub fn apply_performance_tweaks(&self) -> Result<()> {
        // sysctl optimizations
        let sysctl = r#"# ShobikaOs Performance Tweaks
# Network
net.core.netdev_max_backlog = 16384
net.core.somaxconn = 8192
net.ipv4.tcp_fastopen = 3
net.ipv4.tcp_congestion_control = bbr
net.ipv4.tcp_mtu_probing = 1

# VM/Memory
vm.swappiness = 10
vm.vfs_cache_pressure = 50
vm.dirty_ratio = 10
vm.dirty_background_ratio = 5
vm.max_map_count = 2147483642

# FS
fs.inotify.max_user_watches = 524288
fs.inotify.max_user_instances = 1024
fs.file-max = 2097152
"#;
        fs::create_dir_all(&format!("{}/etc/sysctl.d", self.mount_point))?;
        fs::write(&format!("{}/etc/sysctl.d/99-shobikaos.conf", self.mount_point), sysctl)?;
        
        // systemd optimizations
        let system_conf = r#"[Manager]
DefaultTimeoutStopSec=10s
DefaultTimeoutStartSec=30s
"#;
        fs::create_dir_all(&format!("{}/etc/systemd/system.conf.d", self.mount_point))?;
        fs::write(&format!("{}/etc/systemd/system.conf.d/shobikaos.conf", self.mount_point), system_conf)?;
        
        // Makepkg optimizations
        let makepkg = r#"MAKEFLAGS="-j$(nproc)"
COMPRESSZST=(zstd -c -T0 --ultra -20 -)
COMPRESSXZ=(xz -c -z --threads=0 -)
COMPRESSGZ=(pigz -c -f -n)
BUILDDIR=/tmp/makepkg
"#;
        fs::create_dir_all(&format!("{}/etc/makepkg.conf.d", self.mount_point))?;
        fs::write(&format!("{}/etc/makepkg.conf.d/shobikaos.conf", self.mount_point), makepkg)?;
        
        // Pacman optimizations
        let pacman_conf_path = format!("{}/etc/pacman.conf", self.mount_point);
        if let Ok(mut conf) = fs::read_to_string(&pacman_conf_path) {
            // Enable Color, ParallelDownloads, ILoveCandy, VerbosePkgLists
            conf = conf.replace("#Color", "Color");
            conf = conf.replace("#VerbosePkgLists", "VerbosePkgLists");
            conf = conf.replace("#ParallelDownloads = 5", "ParallelDownloads = 10\nILoveCandy");
            
            // Enable multilib
            conf = conf.replace("#[multilib]", "[multilib]");
            conf = conf.replace("#Include = /etc/pacman.d/mirrorlist", "Include = /etc/pacman.d/mirrorlist");
            
            fs::write(&pacman_conf_path, conf)?;
        }
        
        // Enable systemd-oomd
        self.chroot("systemctl enable systemd-oomd")?;
        
        // Enable irqbalance
        let irq_pkgs: Vec<&str> = vec![&self.mount_point, "irqbalance"];
        Command::new("pacstrap").args(&irq_pkgs).status()?;
        self.chroot("systemctl enable irqbalance")?;
        
        // Setup zram
        let zram_pkgs: Vec<&str> = vec![&self.mount_point, "zram-generator"];
        Command::new("pacstrap").args(&zram_pkgs).status()?;
        let zram_conf = r#"[zram0]
zram-size = min(ram / 2, 8192)
compression-algorithm = zstd
"#;
        fs::create_dir_all(&format!("{}/etc/systemd/zram-generator.conf.d", self.mount_point))?;
        fs::write(&format!("{}/etc/systemd/zram-generator.conf", self.mount_point), zram_conf)?;
        
        // Reflector for fast mirrors
        let reflector_pkgs: Vec<&str> = vec![&self.mount_point, "reflector"];
        Command::new("pacstrap").args(&reflector_pkgs).status()?;
        let reflector_conf = r#"--save /etc/pacman.d/mirrorlist
--protocol https
--latest 10
--sort rate
"#;
        fs::write(&format!("{}/etc/xdg/reflector/reflector.conf", self.mount_point), reflector_conf).ok();
        self.chroot("systemctl enable reflector.timer")?;
        
        Ok(())
    }

    pub fn apply_wallpaper(&self) -> Result<()> {
        let wallpaper = &self.config.wallpaper;
        let bg_path = format!("/usr/share/backgrounds/{}", wallpaper);
        
        // Set wallpaper for Hyprpaper
        let hyprpaper_conf = format!("# ShobikaOs Hyprpaper Config\npreload = {}\nwallpaper = ,{}\nsplash = false\nipc = on\n", bg_path, bg_path);
        let user_hyprpaper = format!("{}/home/{}/.config/hypr/hyprpaper.conf", self.mount_point, self.config.username);
        fs::create_dir_all(&format!("{}/home/{}/.config/hypr", self.mount_point, self.config.username))?;
        fs::write(&user_hyprpaper, hyprpaper_conf)?;
        
        // Set as SDDM background
        let sddm_bg = format!("{}/usr/share/sddm/themes/shobikaos/background.png", self.mount_point);
        if std::path::Path::new(&format!("{}{}", self.mount_point, bg_path)).exists() {
            fs::copy(&format!("{}{}", self.mount_point, bg_path), &sddm_bg).ok();
        }
        
        // Set as Plymouth background
        let plymouth_bg = format!("{}/usr/share/plymouth/themes/shobikaos/background.png", self.mount_point);
        if std::path::Path::new(&format!("{}{}", self.mount_point, bg_path)).exists() {
            fs::copy(&format!("{}{}", self.mount_point, bg_path), &plymouth_bg).ok();
        }
        
        // Fix ownership
        self.chroot(&format!("chown -R {}:{} /home/{}", self.config.username, self.config.username, self.config.username))?;
        
        Ok(())
    }

    fn chroot(&self, cmd: &str) -> Result<()> {
        Command::new("arch-chroot")
            .args([&self.mount_point, "bash", "-c", cmd])
            .status()?;
        Ok(())
    }
}