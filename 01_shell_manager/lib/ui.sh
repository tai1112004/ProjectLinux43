#!/usr/bin/env bash

NC='\033[0m'
CYAN='\033[36m'
GREEN='\033[32m'
RED='\033[31m'
YELLOW='\033[33m'
BOLD='\033[1m'

print_header() {
    local title="$1"
    local now
    now="$(date '+%Y-%m-%d %H:%M:%S')"
    local line=" ${title} | ${now} "
    local len=${#line}
    printf "${CYAN}${BOLD}┌"
    printf '─%.0s' $(seq 1 "$len")
    printf "┐\n│%s│\n└" "$line"
    printf '─%.0s' $(seq 1 "$len")
    printf "┘${NC}\n"
}

print_ok() { printf "${GREEN}[✓] %s${NC}\n" "$1"; }
print_err() { printf "${RED}[✗] %s${NC}\n" "$1" >&2; }
print_info() { printf "${YELLOW}[i] %s${NC}\n" "$1"; }

confirm() {
    local answer
    read -r -p "$(printf "${YELLOW}? %s [Y/n]: ${NC}" "$1")" answer
    case "$answer" in
        ""|[Yy]|[Yy][Ee][Ss]) return 0 ;;
        *) return 1 ;;
    esac
}

pause_screen() {
    read -r -p "$(printf "${CYAN}Nhan Enter de tiep tuc...${NC}")"
}

spinner() {
    local pid="$1"
    local msg="${2:-Dang xu ly}"
    local spin='|/-\'
    local i=0
    while kill -0 "$pid" 2>/dev/null; do
        i=$(( (i + 1) % 4 ))
        printf "\r${YELLOW}%s %s${NC}" "$msg" "${spin:$i:1}"
        sleep 0.1
    done
    printf "\r"
}
