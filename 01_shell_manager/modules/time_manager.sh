#!/usr/bin/env bash
set -u
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$BASE_DIR/lib/ui.sh"

show_time() {
    date '+System time : %Y-%m-%d %H:%M:%S %Z'
    hwclock 2>/dev/null || print_info "Khong doc duoc hardware clock neu thieu quyen"
    timedatectl 2>/dev/null | sed 's/^/  /'
}

while true; do
    clear; print_header "TIME MANAGER"
    echo "1) Hien thi thoi gian/timezone/NTP"; echo "2) Dat timezone"; echo "3) Bat dong bo NTP"
    echo "4) Dat gio thu cong"; echo "5) Kiem tra NTP sync"; echo "0) Ve menu chinh"
    read -r -p "Chon: " c
    case "$c" in
        1) show_time; pause_screen ;;
        2) read -r -p "Timezone (vd Asia/Ho_Chi_Minh): " tz; confirm "Dat timezone $tz?" && sudo timedatectl set-timezone "$tz"; pause_screen ;;
        3) confirm "Bat NTP sync?" && sudo timedatectl set-ntp true && print_ok "Da bat NTP"; pause_screen ;;
        4) read -r -p "Thoi gian YYYY-MM-DD HH:MM:SS: " t; confirm "Dat gio '$t'?" && sudo timedatectl set-time "$t"; pause_screen ;;
        5) timedatectl show -p NTPSynchronized -p NTP -p TimeUSec 2>/dev/null; pause_screen ;;
        0) exit 0 ;;
        *) print_err "Lua chon khong hop le"; sleep 1 ;;
    esac
done
