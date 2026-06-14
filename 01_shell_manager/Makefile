.PHONY: all run check clean

all: check

check:
	@bash -n main.sh
	@bash -n lib/ui.sh
	@bash -n modules/file_manager.sh
	@bash -n modules/scheduler.sh
	@bash -n modules/time_manager.sh
	@bash -n modules/pkg_manager.sh
	@echo "Shell scripts syntax OK"

run:
	@bash main.sh

clean:
	@true
