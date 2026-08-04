# ShobikaOs Zsh Configuration
# ~/.zshrc

# Enable Powerlevel10k instant prompt
if [[ -r "${XDG_CACHE_HOME:-$HOME/.cache}/p10k-instant-prompt-${(%):-%n}.zsh" ]]; then
  source "${XDG_CACHE_HOME:-$HOME/.cache}/p10k-instant-prompt-${(%):-%n}.zsh"
fi

# Oh My Zsh
export ZSH="$HOME/.oh-my-zsh"
ZSH_THEME="powerlevel10k/powerlevel10k"

plugins=(
    git
    sudo
    archlinux
    systemd
    docker
    docker-compose
    kubectl
    rust
    cargo
    golang
    node
    npm
    python
    pip
    virtualenv
    fzf
    zsh-autosuggestions
    zsh-syntax-highlighting
    zsh-completions
    history-substring-search
    direnv
    starship
)

source $ZSH/oh-my-zsh.sh

# User configuration
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
export EDITOR=nvim
export VISUAL=nvim
export SUDO_EDITOR=nvim
export PAGER=less
export LESS='-R -F -X'

# Path
export PATH="$HOME/.local/bin:$HOME/.cargo/bin:$HOME/go/bin:$PATH"

# ShobikaOs branding
export DISTRO_NAME="ShobikaOs"
export DISTRO_VERSION="rolling"
export DISTRO_URL="https://github.com/shobikaos/ShobikaOs"

# Aliases
alias ls='ls --color=auto -h'
alias ll='ls -la'
alias la='ls -A'
alias l='ls -CF'
alias grep='grep --color=auto'
alias diff='diff --color=auto'
alias ip='ip --color=auto'
alias rm='rm -i'
alias cp='cp -i'
alias mv='mv -i'
alias ..='cd ..'
alias ...='cd ../..'
alias c='clear'
alias h='history'
alias update='sudo pacman -Syu'
alias install='sudo pacman -S'
alias remove='sudo pacman -Rs'
alias search='pacman -Ss'
alias clean='sudo pacman -Sc'
alias mirror='sudo reflector --latest 10 --sort rate --save /etc/pacman.d/mirrorlist'
alias g='git'
alias ga='git add'
alias gc='git commit -m'
alias gp='git push'
alias gs='git status'
alias gd='git diff'
alias d='docker'
alias dc='docker compose'

# Fastfetch
if [[ -t 0 ]] && command -v fastfetch &> /dev/null; then
    fastfetch --config ~/.config/fastfetch/config.jsonc
fi

# Starship (if not using p10k)
# eval "$(starship init zsh)"

# FZF
source /usr/share/fzf/key-bindings.zsh
source /usr/share/fzf/completion.zsh

# Direnv
eval "$(direnv hook zsh)"

# To customize prompt, run `p10k configure` or edit ~/.p10k.zsh
[[ ! -f ~/.p10k.zsh ]] || source ~/.p10k.zsh

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
command -v zoxide &> /dev/null && eval "$(zoxide init zsh)"
command -v fd &> /dev/null && alias find='fd'
command -v rg &> /dev/null && alias grep='rg'

# Systemd shortcuts
alias sc='sudo systemctl'
alias scu='systemctl --user'
alias jc='journalctl -xe'
alias reload-daemon='sudo systemctl daemon-reload'

# Welcome
echo -e "\e[38;5;117mWelcome to \e[1mShobikaOs\e[0m\e[38;5;117m - Arch-based Linux Distribution\e[0m"
echo -e "\e[38;5;103mRepository: \e[4mhttps://github.com/shobikaos/ShobikaOs\e[0m"
echo ""