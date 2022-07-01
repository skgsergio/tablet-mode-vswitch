PREFIX ?=

all:
	make -C src

install: all
	make -C src install
	install -m 644 systemd/tablet-mode-vswitch.service $(PREFIX)/usr/lib/systemd/system/
