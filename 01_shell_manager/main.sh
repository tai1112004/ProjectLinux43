#!/usr/bin/env bash
set -u

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib/ui.sh
source "$BASE_DIR/lib/ui.sh"

while true; do
    clear
    print_header "LINUX SYSTEM MANAGER"
    echo "  1) File manager"
    echo "  2) Cron scheduler"
    echo "  3) Time manager"
    echo "  4) Package manager"
    echo "  0) Thoat"
    printf "\n"
    read -r -p "Chon: " choice
    case "$choice" in
        1) bash "$BASE_DIR/modules/file_manager.sh" ;;
        2) bash "$BASE_DIR/modules/scheduler.sh" ;;
        3) bash "$BASE_DIR/modules/time_manager.sh" ;;
        4) bash "$BASE_DIR/modules/pkg_manager.sh" ;;
        0) print_ok "Tam biet"; exit 0 ;;
        *) print_err "Lua chon khong hop le"; sleep 1 ;;
    esac
done
