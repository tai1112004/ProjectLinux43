#!/usr/bin/env bash
set -euo pipefail

NC='\033[0m'; CYAN='\033[36m'; GREEN='\033[32m'; RED='\033[31m'; YELLOW='\033[33m'
info() { printf "${YELLOW}[i] %s${NC}\n" "$1"; }
ok() { printf "${GREEN}[✓] %s${NC}\n" "$1"; }
err() { printf "${RED}[✗] %s${NC}\n" "$1" >&2; }

printf "${CYAN}┌──────────────────────────────┐\n│ HELLO MODULE TEST            │\n└──────────────────────────────┘${NC}\n"
info "Build module"; make
info "Load module"; sudo insmod hello_module.ko || { err "insmod failed"; exit 1; }
ok "Loaded"
info "lsmod verify"; lsmod | grep hello_module || true
info "cat /proc/hello_info"; cat /proc/hello_info
info "dmesg tail"; sudo dmesg | tail -20
info "Unload module"; sudo rmmod hello_module
ok "Unloaded"
