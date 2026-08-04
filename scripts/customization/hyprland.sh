#!/bin/bash
# ShobikaOs - Hyprland customization
set -e

echo "[ShobikaOs] Customizing Hyprland..."

mkdir -p ~/.config/hypr
mkdir -p ~/.config/waybar
mkdir -p ~/.config/wofi
mkdir -p ~/.config/foot

# Hyprland config
cat > ~/.config/hypr/hyprland.conf << 'EOF'
# ShobikaOs Hyprland Config

monitor=,preferred,auto,1

general {
    gaps_in = 5
    gaps_out = 10
    border_size = 2
    col.active_border = rgba(5eacd3ee) rgba(b48eadee) 45deg
    col.inactive_border = rgba(595959aa)
    layout = dwindle
}

decoration {
    rounding = 10
    
    blur {
        enabled = true
        size = 3
        passes = 1
    }
    
    shadow {
        enabled = true
        range = 4
        render_power = 3
        color = rgba(1a1a1aee)
    }
}

animations {
    enabled = true
    bezier = snazzy, 0.25, 1, 0.5, 1
    animation = windows, 1, 7, snazzy
    animation = windowsOut, 1, 7, default, popin 80%
    animation = border, 1, 10, default
    animation = borderangle, 1, 8, default
    animation = fade, 1, 7, default
    animation = workspaces, 1, 6, default
}

dwindle {
    pseudotile = true
    preserve_split = true
}

input {
    kb_layout = us
    follow_mouse = 1
    touchpad {
        natural_scroll = true
    }
}

$mod = SUPER

bind = $mod, Return, exec, foot
bind = $mod, D, exec, wofi --show drun
bind = $mod, Q, killactive
bind = $mod, F, fullscreen
bind = $mod, V, togglefloating
bind = $mod, M, exit

bind = $mod, H, movefocus, l
bind = $mod, L, movefocus, r
bind = $mod, K, movefocus, u
bind = $mod, J, movefocus, d

bind = $mod, 1, workspace, 1
bind = $mod, 2, workspace, 2
bind = $mod, 3, workspace, 3
bind = $mod, 4, workspace, 4
bind = $mod, 5, workspace, 5

bind = $mod SHIFT, 1, movetoworkspace, 1
bind = $mod SHIFT, 2, movetoworkspace, 2
bind = $mod SHIFT, 3, movetoworkspace, 3
bind = $mod SHIFT, 4, movetoworkspace, 4
bind = $mod SHIFT, 5, movetoworkspace, 5

exec-once = waybar
exec-once = hyprpaper
exec-once = dunst
exec-once = /usr/lib/polkit-gnome/polkit-gnome-authentication-agent-1
exec-once = nm-applet --indicator
EOF

# Waybar config
cat > ~/.config/waybar/config << 'EOF'
{
    "layer": "top",
    "position": "bottom",
    "height": 30,
    "modules-left": ["hyprland/workspaces"],
    "modules-center": ["hyprland/window"],
    "modules-right": ["tray", "network", "pulseaudio", "battery", "clock"],
    "hyprland/workspaces": {
        "format": "{id}",
        "on-click": "activate"
    },
    "hyprland/window": {
        "format": "{}"
    },
    "tray": {
        "icon-size": 16,
        "spacing": 10
    },
    "network": {
        "format-wifi": " {signalStrength}%",
        "format-ethernet": " {ifname}",
        "format-disconnected": "Offline",
        "tooltip-format": "{ifname}: {ipaddr}/{cidr}"
    },
    "pulseaudio": {
        "format": "{icon} {volume}%",
        "format-icons": {
            "default": ["", "", ""]
        },
        "on-click": "pavucontrol"
    },
    "battery": {
        "format": "{icon} {capacity}%",
        "format-icons": ["", "", "", "", ""],
        "states": {
            "warning": 30,
            "critical": 15
        }
    },
    "clock": {
        "format": "{:%H:%M}",
        "tooltip-format": "{:%Y-%m-%d}"
    }
}
EOF

# Waybar style
cat > ~/.config/waybar/style.css << 'EOF'
* {
    font-family: "JetBrains Mono Nerd Font", monospace;
    font-size: 13px;
}

window#waybar {
    background-color: rgba(46, 52, 64, 0.9);
    color: #d8dee9;
    border-top: 2px solid #5eacd3;
}

#workspaces button {
    padding: 0 5px;
    color: #d8dee9;
    background-color: transparent;
    border-radius: 5px;
}

#workspaces button.active {
    background-color: #5eacd3;
    color: #2e3440;
}

#tray, #network, #pulseaudio, #battery, #clock {
    padding: 0 10px;
}
EOF

# Foot terminal config
cat > ~/.config/foot/foot.ini << 'EOF'
[main]
font=JetBrains Mono:size=11
dpi-aware=yes

[colors]
background=2e3440
foreground=d8dee9

regular0=3b4252
regular1=bf616a
regular2=a3be8c
regular3=ebcb8b
regular4=81a1c1
regular5=b48ead
regular6=88c0d0
regular7=e5e9f0
EOF

# Wofi config
cat > ~/.config/wofi/config << 'EOF'
width=600
height=400
location=center
show=drun
prompt=Search...
filter_rate=100
allow_markup=true
no_actions=true
halign=fill
orientation=vertical
content_halign=fill
insensitive=true
allow_images=true
image_size=24
gtk_dark=true
EOF

# Wallpaper
cp /usr/share/backgrounds/shobikaos-wallpaper.png ~/.config/hypr/wallpaper.png
cat >> ~/.config/hypr/hyprland.conf << 'EOF'

exec-once = hyprpaper
EOF

cat > ~/.config/hypr/hyprpaper.conf << 'EOF'
preload = ~/.config/hypr/wallpaper.png
wallpaper = ,~/.config/hypr/wallpaper.png
EOF

echo "[ShobikaOs] Hyprland customized!"
