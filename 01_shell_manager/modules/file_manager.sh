#!/usr/bin/env bash
set -u
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$BASE_DIR/lib/ui.sh"

show_table() {
    local n=1
    printf "${CYAN}%-6s | %s${NC}\n" "STT" "Ket qua"
    printf '%s\n' "-------+------------------------------------------------------------"
    while IFS= read -r line; do
        printf "%-6d | %s\n" "$n" "$line"
        n=$((n + 1))
    done
}

copy_move() {
    local op="$1" src dst
    read -r -p "Nguon: " src
    read -r -p "Dich: " dst
    if [[ ! -e "$src" ]]; then print_err "Nguon khong ton tai"; return; fi
    if command -v rsync >/dev/null 2>&1; then
        [[ "$op" == "copy" ]] && rsync -ah --progress "$src" "$dst" || rsync -ah --progress --remove-source-files "$src" "$dst"
    else
        [[ "$op" == "copy" ]] && cp -rv "$src" "$dst" || mv -v "$src" "$dst"
    fi
}

while true; do
    clear; print_header "FILE MANAGER"
    echo "1) Tao thu muc"; echo "2) Tao file"; echo "3) Xoa"; echo "4) Copy"
    echo "5) Move"; echo "6) Tim theo ten"; echo "7) Tim theo noi dung"; echo "0) Ve menu chinh"
    read -r -p "Chon: " c
    case "$c" in
        1) read -r -p "Duong dan thu muc: " p; mkdir -p "$p" && print_ok "Da tao $p"; pause_screen ;;
        2) read -r -p "Duong dan file: " p; touch "$p" && print_ok "Da tao $p"; pause_screen ;;
        3) read -r -p "Duong dan can xoa: " p; confirm "Xoa '$p'?" && rm -rf "$p" && print_ok "Da xoa"; pause_screen ;;
        4) copy_move copy; pause_screen ;;
        5) copy_move move; pause_screen ;;
        6) read -r -p "Pattern name: " p; find . -name "$p" 2>/dev/null | show_table; pause_screen ;;
        7) read -r -p "Keyword: " k; read -r -p "Path [.]: " p; p=${p:-.}; grep -rn -- "$k" "$p" 2>/dev/null | show_table; pause_screen ;;
        0) exit 0 ;;
        *) print_err "Lua chon khong hop le"; sleep 1 ;;
    esac
done
