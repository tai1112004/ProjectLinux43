.PHONY: all shell system kernel driver gui clean help

all: shell system kernel driver

shell:
	$(MAKE) -C 01_shell_manager

system:
	$(MAKE) -C 02_system_prog

kernel:
	$(MAKE) -C 03_kernel_module

driver:
	$(MAKE) -C 04_char_driver

gui:
	python3 gui_app/app.py

clean:
	$(MAKE) -C 02_system_prog clean
	$(MAKE) -C 03_kernel_module clean || true
	$(MAKE) -C 04_char_driver clean || true

help:
	@echo "Targets: all shell system kernel driver gui clean"
