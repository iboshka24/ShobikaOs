use anyhow::Result;
use std::process::Command;

#[derive(Debug, Clone)]
pub enum GpuVendor {
    Nvidia,
    Amd,
    Intel,
    Unknown,
}

#[derive(Debug, Clone)]
pub enum CpuVendor {
    Intel,
    Amd,
    Unknown,
}

#[derive(Debug, Clone)]
pub struct HardwareInfo {
    pub gpu: GpuVendor,
    pub cpu: CpuVendor,
    pub has_bluetooth: bool,
    pub has_broadcom_wifi: bool,
    pub is_laptop: bool,
}

impl HardwareInfo {
    pub fn detect() -> Self {
        Self {
            gpu: Self::detect_gpu(),
            cpu: Self::detect_cpu(),
            has_bluetooth: Self::check_bluetooth(),
            has_broadcom_wifi: Self::check_broadcom(),
            is_laptop: Self::check_laptop(),
        }
    }

    fn detect_gpu() -> GpuVendor {
        let output = Command::new("lspci")
            .args(["-nn"])
            .output()
            .map(|o| String::from_utf8_lossy(&o.stdout).to_lowercase())
            .unwrap_or_default();
        
        if output.contains("nvidia") {
            GpuVendor::Nvidia
        } else if output.contains("amd") || output.contains("radeon") || output.contains("advanced micro") {
            GpuVendor::Amd
        } else if output.contains("intel") {
            GpuVendor::Intel
        } else {
            GpuVendor::Unknown
        }
    }

    fn detect_cpu() -> CpuVendor {
        let output = Command::new("cat")
            .arg("/proc/cpuinfo")
            .output()
            .map(|o| String::from_utf8_lossy(&o.stdout).to_lowercase())
            .unwrap_or_default();
        
        if output.contains("genuineintel") || output.contains("intel") {
            CpuVendor::Intel
        } else if output.contains("authenticamd") || output.contains("amd") {
            CpuVendor::Amd
        } else {
            CpuVendor::Unknown
        }
    }

    fn check_bluetooth() -> bool {
        Command::new("lsusb")
            .output()
            .map(|o| String::from_utf8_lossy(&o.stdout).to_lowercase().contains("bluetooth"))
            .unwrap_or(false)
        || Command::new("lspci")
            .output()
            .map(|o| String::from_utf8_lossy(&o.stdout).to_lowercase().contains("bluetooth"))
            .unwrap_or(false)
    }

    fn check_broadcom() -> bool {
        Command::new("lspci")
            .output()
            .map(|o| String::from_utf8_lossy(&o.stdout).to_lowercase().contains("broadcom"))
            .unwrap_or(false)
    }

    fn check_laptop() -> bool {
        std::path::Path::new("/sys/class/power_supply/BAT0").exists()
        || std::path::Path::new("/sys/class/power_supply/BAT1").exists()
    }

    pub fn gpu_packages(&self) -> Vec<&'static str> {
        match self.gpu {
            GpuVendor::Nvidia => vec![
                "nvidia-dkms", "nvidia-utils", "lib32-nvidia-utils",
                "nvidia-settings", "opencl-nvidia", "libva-nvidia-driver",
                "linux-headers",
            ],
            GpuVendor::Amd => vec![
                "mesa", "vulkan-radeon", "lib32-vulkan-radeon",
                "lib32-mesa", "libva-mesa-driver", "mesa-vdpau",
                "xf86-video-amdgpu",
            ],
            GpuVendor::Intel => vec![
                "mesa", "vulkan-intel", "lib32-vulkan-intel",
                "lib32-mesa", "intel-media-driver", "libva-intel-driver",
            ],
            GpuVendor::Unknown => vec!["mesa", "lib32-mesa"],
        }
    }

    pub fn cpu_packages(&self) -> Vec<&'static str> {
        match self.cpu {
            CpuVendor::Intel => vec!["intel-ucode"],
            CpuVendor::Amd => vec!["amd-ucode"],
            CpuVendor::Unknown => vec![],
        }
    }

    pub fn extra_packages(&self) -> Vec<&'static str> {
        let mut pkgs = vec!["xf86-input-libinput", "libinput"];
        
        if self.has_bluetooth {
            pkgs.extend_from_slice(&["bluez", "bluez-utils", "blueman"]);
        }
        if self.has_broadcom_wifi {
            pkgs.push("broadcom-wl-dkms");
        }
        if self.is_laptop {
            pkgs.extend_from_slice(&["tlp", "tlp-rdw", "acpi", "acpid"]);
        }
        
        pkgs
    }
}
