CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -g
LDFLAGS ?=
SRC := $(wildcard src/*.c)
BINS := $(patsubst src/%.c,bin/%,$(SRC))

.PHONY: all clean run

all: $(BINS)

bin/%: src/%.c | bin
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -pthread

bin:
	mkdir -p bin

run: all
	./bin/main_demo

clean:
	rm -f bin/*
