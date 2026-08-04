use anyhow::Result;
use std::process::Command;

pub fn get_disks() -> Result<Vec<DiskInfo>> {
    let output = Command::new("lsblk")
        .args(["-J", "-o", "NAME,SIZE,TYPE,MODEL,TRAN"])
        .output()?;
    
    #[derive(serde::Deserialize)]
    struct LsblkOutput {
        blockdevices: Vec<LsblkDevice>,
    }
    
    #[derive(serde::Deserialize)]
    struct LsblkDevice {
        name: String,
        size: String,
        #[serde(rename = "type")]
        dev_type: String,
        model: Option<String>,
        tran: Option<String>,
        children: Option<Vec<LsblkDevice>>,
    }
    
    let parsed: LsblkOutput = serde_json::from_slice(&output.stdout)?;
    let mut disks = Vec::new();
    
    for dev in parsed.blockdevices {
        if dev.dev_type == "disk" {
            let size_bytes = parse_size(&dev.size)?;
            disks.push(DiskInfo {
                path: format!("/dev/{}", dev.name),
                size: size_bytes,
                model: dev.model.unwrap_or_default(),
                transport: dev.tran.unwrap_or_default(),
            });
        }
    }
    
    Ok(disks)
}

#[derive(Debug, Clone)]
pub struct DiskInfo {
    pub path: String,
    pub size: u64,
    pub model: String,
    pub transport: String,
}

fn parse_size(s: &str) -> Result<u64> {
    let s = s.trim();
    let (num, unit) = if s.ends_with("G") || s.ends_with("M") || s.ends_with("K") || s.ends_with("T") {
        (&s[..s.len()-1], &s[s.len()-1..])
    } else {
        (s, "")
    };
    
    let num: f64 = num.parse()?;
    let mult = match unit {
        "K" => 1024.0,
        "M" => 1024.0 * 1024.0,
        "G" => 1024.0 * 1024.0 * 1024.0,
        "T" => 1024.0 * 1024.0 * 1024.0 * 1024.0,
        _ => 1.0,
    };
    
    Ok((num * mult) as u64)
}