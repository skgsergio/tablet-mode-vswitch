CC ?= gcc
CFLAGS ?=
LDFLAGS ?=

PREFIX ?=

CFLAGS += $(shell pkg-config --cflags libevdev libudev)
LDFLAGS += $(shell pkg-config --libs libevdev libudev)

BIN = tablet-mode-vswitch

all: $(BIN)

debug:
debug: CFLAGS += -DDEBUG
debug: $(BIN)

.c:
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

install: all
	install -m 755 $(BIN) $(PREFIX)/usr/bin

clean:
	rm -f $(BIN)
