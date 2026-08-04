# ShobikaOs Bash Configuration
# ~/.bashrc

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

# History
HISTCONTROL=ignoreboth
HISTSIZE=10000
HISTFILESIZE=20000
shopt -s histappend
shopt -s checkwinsize

# Colors
alias ls='ls --color=auto -h'
alias ll='ls -la'
alias la='ls -A'
alias l='ls -CF'
alias grep='grep --color=auto'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'
alias diff='diff --color=auto'
alias ip='ip --color=auto'

# Safety
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'

# Shortcuts
alias ..='cd ..'
alias ...='cd ../..'
alias ....='cd ../../..'
alias ~='cd ~'
alias c='clear'
alias h='history'
alias j='jobs -l'
alias path='echo -e ${PATH//:/\\n}'
alias ports='ss -tulpn'

# System
alias update='sudo pacman -Syu'
alias install='sudo pacman -S'
alias remove='sudo pacman -Rs'
alias search='pacman -Ss'
alias orphans='pacman -Qdtq'
alias clean='sudo pacman -Sc'
alias mirror='sudo reflector --latest 10 --sort rate --save /etc/pacman.d/mirrorlist'

# Git
alias g='git'
alias ga='git add'
alias gc='git commit -m'
alias gp='git push'
alias gl='git log --oneline -10'
alias gs='git status'
alias gd='git diff'
alias gb='git branch'
alias gco='git checkout'

# Docker/Podman
alias d='docker'
alias dc='docker compose'
alias dps='docker ps'
alias di='docker images'

# ShobikaOs branding
export DISTRO_NAME="ShobikaOs"
export DISTRO_VERSION="rolling"
export DISTRO_URL="https://github.com/shobikaos/ShobikaOs"

# Prompt
if [[ $EUID -eq 0 ]]; then
    PS1='\[\e[38;5;203m\]\u\[\e[0m\]@\[\e[38;5;117m\]\h\[\e[0m\]:\[\e[38;5;75m\]\w\[\e[0m\]\n\[\e[38;5;203m\]#\[\e[0m\] '
else
    PS1='\[\e[38;5;117m\]\u\[\e[0m\]@\[\e[38;5;75m\]\h\[\e[0m\]:\[\e[38;5;117m\]\w\[\e[0m\]\n\[\e[38;5;117m\]\$\[\e[0m\] '
fi

# Fastfetch on login (only in interactive shells, not in scripts)
if [[ -t 0 ]] && [[ -o interactive ]] && command -v fastfetch &> /dev/null; then
    fastfetch --config ~/.config/fastfetch/config.jsonc
fi

# Completion
if [[ -f /usr/share/bash-completion/bash_completion ]]; then
    . /usr/share/bash-completion/bash_completion
fi

# FZF
if [[ -f /usr/share/fzf/key-bindings.bash ]]; then
    source /usr/share/fzf/key-bindings.bash
fi
if [[ -f /usr/share/fzf/completion.bash ]]; then
    source /usr/share/fzf/completion.bash
fi

# Direnv
if command -v direnv &> /dev/null; then
    eval "$(direnv hook bash)"
fi

# Starship (if installed)
if command -v starship &> /dev/null; then
    eval "$(starship init bash)"
fi

# Local bin
export PATH="$HOME/.local/bin:$PATH"

# Editor
export EDITOR=nvim
export VISUAL=nvim
export SUDO_EDITOR=nvim

# Pager
export PAGER=less
export LESS='-R -F -X'

# Locale
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

# Qt
export QT_QPA_PLATFORMTHEME=gtk2
export QT_STYLE_OVERRIDE=gtk2

# GTK
export GTK_THEME=ShobikaOs-Dark
export GTK2_RC_FILES="$HOME/.gtkrc-2.0"

# Wayland
export MOZ_ENABLE_WAYLAND=1
export QT_QPA_PLATFORM=wayland
export SDL_VIDEODRIVER=wayland
export CLUTTER_BACKEND=wayland

# Java
export _JAVA_AWT_WM_NONREPARENTING=1

# Rust
export CARGO_HOME="$HOME/.cargo"
export RUSTUP_HOME="$HOME/.rustup"
export PATH="$CARGO_HOME/bin:$PATH"

# Go
export GOPATH="$HOME/go"
export PATH="$GOPATH/bin:$PATH"

# Node (fnm)
if command -v fnm &> /dev/null; then
    eval "$(fnm env --use-on-cd)"
fi

# Python
export PYTHONDONTWRITEBYTECODE=1
export PYTHONPYCACHEPREFIX="$HOME/.cache/python"
export STEAM_FORCE_DESKTOPUI_SCALING=1

# Gaming
alias steam-run='gamemoderun steam'
alias mangohud-run='mangohud --dlsym'
alias gamemode-test='gamemoded -t'
alias proton-tricks='protontricks'

# Modern CLI replacements (if installed)
command -v eza &> /dev/null && alias ls='eza --icons --group-directories-first'
command -v eza &> /dev/null && alias ll='eza -la --icons --group-directories-first'
command -v eza &> /dev/null && alias la='eza -a --icons --group-directories-first'
command -v eza &> /dev/null && alias tree='eza --tree --icons'
command -v bat &> /dev/null && alias cat='bat --style=auto'
command -v zoxide &> /dev/null && eval "$(zoxide init bash)"
command -v fd &> /dev/null && alias find='fd'
command -v rg &> /dev/null && alias grep='rg'

# Systemd shortcuts
alias sc='sudo systemctl'
alias scu='systemctl --user'
alias jc='journalctl -xe'
alias reload-daemon='sudo systemctl daemon-reload'

# Welcome message
echo -e "\e[38;5;117mWelcome to \e[1mShobikaOs\e[0m\e[38;5;117m - Arch-based Linux Distribution\e[0m"
echo -e "\e[38;5;103mRepository: \e[4mhttps://github.com/shobikaos/ShobikaOs\e[0m"
echo ""