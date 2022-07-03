# Tablet Mode virtual switch

This project provides a daemon that acts as a Tablet Mode virtual switch that
toggles on HID device connect and disconnect.

I made it since I was using `ydotool` which provides a virtual pointer device
preventing GNOME to toggle tablet mode on my Microsoft Surface Pro (2017)
aka 5th gen where the device reports itself as a Laptop and the Tablet Mode
switch is present in the Microsoft Surface Type Cover.

GNOME toggles tablet mode when there are no pointers but `ydotool` provides
one.

## How it works

This daemon provides a virtual HID device which has a Tablet Mode switch. This
switch is toggled to true when a HID device included in the internal list is
disconnected and to false when it is connected.

## How to Install

Clone this repo and run:

```shell
make
sudo make install
```

## Enabling the service

There is a ready to go systemd service unit you just have to enable and start it
as follows:

```shell
sudo systemctl enable tablet-mode-vswitch.service
sudo systemctl start tablet-mode-vswitch.service
```

If you want to check daemon logs just use `journalctl`:

```shell
sudo journalctl -u tablet-mode-vswitch
```

## To DO list

- Detect whether the HID devices in the list are present or not when the daemon
  starts and set the virtual Tablet Mode switch accordingly. Currently starts as
  false.

## License

This project is licensed under GPL3 or later license which you can find in the
[LICENSE](./LICENSE) file.
