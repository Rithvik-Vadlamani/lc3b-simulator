CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra

all: build/pipeline build/virtual-memory

build:
	mkdir -p build

build/pipeline: lc3bsim6_3.c | build
	$(CC) $(CFLAGS) $< -o $@

build/virtual-memory: lc3bsim5.c | build
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build

.PHONY: all clean
