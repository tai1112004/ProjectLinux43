#!/usr/bin/env bash
set -u
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$BASE_DIR/lib/ui.sh"

disk_line() { df -h / | awk 'NR==2 {printf "Disk /: used=%s avail=%s\n", $3, $4}'; }
run_with_spinner() {
    "$@" &
    local pid=$!
    spinner "$pid" "Dang chay: $*"
    wait "$pid"
}

while true; do
    clear; print_header "APT PACKAGE MANAGER"
    echo "1) Tim package"; echo "2) Xem thong tin package"; echo "3) Cai package"
    echo "4) Go package purge"; echo "5) Liet ke package da cai"; echo "0) Ve menu chinh"
    read -r -p "Chon: " c
    case "$c" in
        1) read -r -p "Tu khoa: " q; apt-cache search "$q" | head -50; pause_screen ;;
        2) read -r -p "Package: " p; apt-cache show "$p" | sed -n '1,80p'; pause_screen ;;
        3)
            read -r -p "Package can cai: " p
            disk_line
            if confirm "Cai $p bang apt-get install -y?"; then
                run_with_spinner sudo apt-get install -y "$p" && print_ok "Da cai $p"
                disk_line
            fi
            pause_screen ;;
        4)
            read -r -p "Package can go: " p
            disk_line
            if confirm "Go va purge $p?"; then
                run_with_spinner sudo apt-get remove --purge -y "$p" && print_ok "Da go $p"
                disk_line
            fi
            pause_screen ;;
        5) read -r -p "Pattern: " p; dpkg -l | grep --color=always -i "$p" || true; pause_screen ;;
        0) exit 0 ;;
        *) print_err "Lua chon khong hop le"; sleep 1 ;;
    esac
done
