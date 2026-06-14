obj-m += mychardev.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: all modules userspace clean

all: modules userspace

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

userspace: test_app test_blocking

test_app: test_app.c mychardev.h
	$(CC) -Wall -Wextra -O2 -o $@ test_app.c

test_blocking: test_blocking.c mychardev.h
	$(CC) -Wall -Wextra -O2 -pthread -o $@ test_blocking.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f test_app test_blocking
