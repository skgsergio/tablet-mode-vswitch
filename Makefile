CC ?= gcc
CFLAGS ?=
LDFLAGS ?=

CFLAGS += $(shell pkg-config --cflags libevdev libudev)
LDFLAGS += $(shell pkg-config --libs libevdev libudev)

.PHONY: all debug clean

all: tablet-mode-vswitch

debug:
debug: CFLAGS += -DDEBUG
debug: tablet-mode-vswitch

tablet-mode-vswitch.c:
	$(CC) $(CFLAGS) tablet-mode-vswitch.c $(LDFLAGS) -o tablet-mode-vswitch

clean:
	rm tablet-mode-vswitch
