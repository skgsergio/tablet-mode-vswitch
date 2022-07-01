# Tablet Mode virtual switch

This project provides a daemon that acts as a Tablet Mode virtual switch that
toggles on HID device connect and disconnect.

It is designed for resolving the issue I face in a Microsoft Surface Pro (2017)
aka 5th gen where the device reports itself as a Laptop and the Tablet Mode
switch is present in the Microsoft Surface Type Cover. This Tablet Mode switch
only toggles when you fold back the keyboard but if you disconnect it the switch
disappears and the system stays in laptop mode causing the Desktop Environments
to disable tablet features as it is a laptop and there is no tablet switch.

## How it works

This daemon provides a virtual HID device which has a Tablet Mode switch. This
switch is toggled to true when a HID device included in the internal list is
disconnected and to false when it is connected.

## To DO list

- Detect whether the HID devices in the list are present or not when the daemon
  starts and set the virtual Tablet Mode switch accordingly. Currently starts as
  false.

## License

This project is licensed under GPL3 or later license which you can find in the
[LICENSE](./LICENSE) file.
