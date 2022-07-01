CC ?= gcc
CFLAGS ?=
LDFLAGS ?=

CFLAGS += $(shell pkg-config --cflags libevdev libudev)
LDFLAGS += $(shell pkg-config --libs libevdev libudev)

BIN = tablet-mode-vswitch

BINDIR = /usr/bin

all: $(BIN)

debug:
debug: CFLAGS += -DDEBUG
debug: $(BIN)

.c:
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

install: all
	install -m 755 $(BIN) $(BINDIR)

clean:
	rm -f $(BIN)
