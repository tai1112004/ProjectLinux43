#!/usr/bin/env bash
set -u
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$BASE_DIR/lib/ui.sh"

valid_field() { [[ "$1" =~ ^(\*|[0-9]{1,2})$ ]]; }
valid_cron() {
    local expr="$1"
    read -r m h dom mon dow _ <<< "$expr"
    valid_field "$m" && valid_field "$h" && valid_field "$dom" && valid_field "$mon" && valid_field "$dow"
}

list_jobs() {
    local n=1
    crontab -l 2>/dev/null | sed '/^$/d' | while IFS= read -r job; do
        printf "%3d) %s\n" "$n" "$job"; n=$((n + 1))
    done
}

while true; do
    clear; print_header "CRON SCHEDULER"
    echo "1) Xem cron jobs"; echo "2) Them job"; echo "3) Xoa job theo STT"; echo "0) Ve menu chinh"
    read -r -p "Chon: " c
    case "$c" in
        1) list_jobs; pause_screen ;;
        2)
            read -r -p "Phut (* hoac 0-59): " m
            read -r -p "Gio (* hoac 0-23): " h
            read -r -p "Lenh: " cmd
            expr="$m $h * * * $cmd"
            if valid_cron "$expr"; then
                (crontab -l 2>/dev/null; echo "$expr") | crontab - && print_ok "Da them: $expr"
            else
                print_err "Cron expression khong hop le"
            fi
            pause_screen ;;
        3)
            list_jobs
            read -r -p "Nhap STT can xoa: " idx
            tmp="$(mktemp)"
            crontab -l 2>/dev/null | sed '/^$/d' | awk -v del="$idx" 'NR != del' > "$tmp"
            confirm "Luu crontab sau khi xoa job $idx?" && crontab "$tmp" && print_ok "Da cap nhat"
            rm -f "$tmp"
            pause_screen ;;
        0) exit 0 ;;
        *) print_err "Lua chon khong hop le"; sleep 1 ;;
    esac
done
