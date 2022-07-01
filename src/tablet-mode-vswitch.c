/**
* Copyright (C) 2022, Sergio Conde <skgsergio@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
**/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include <sys/poll.h>

#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <libudev.h>

// List of HID_IDs to watch
const char *hid_ids[] = {
    "0003:0000045E:000009C0", // Microsoft Surface Type Cover
#ifdef DEBUG
    "0003:00000416:00000123", // Ducky Ducky One2 SF RGB (my USB keyboard, for testing purposes)
#endif
    NULL,
};

// Handle exit signals
volatile sig_atomic_t exit_signal = 0;

void exit_signal_handler(int signal) {
  fprintf(stdout, "got signal %d, exiting...\n", signal);
  exit_signal = signal;
}

// Tablet mode switch
int set_tablet_mode(struct libevdev_uinput *uinput_dev, bool enable) {
  int ret = 0;

  fprintf(stdout, "%sabling tablet mode switch...\n", enable ? "en" : "dis");

  ret = libevdev_uinput_write_event(uinput_dev, EV_SW, SW_TABLET_MODE, enable ? 0x01 : 0x00);
  if (ret != 0) {
    fprintf(stderr, "libevdev_uinput_wri te_event(uinput_dev, EV_SW, SW_TABLET_MODE, %d) = %s (%d)\n",
            enable ? 0x01 : 0x00, strerror(-ret), ret);
    return ret;
  }

  ret = libevdev_uinput_write_event(uinput_dev, EV_SYN, SYN_REPORT, 0);
  if (ret != 0) {
    fprintf(stderr, "libevdev_uinput_write_event(uinput_dev, EV_SYN, SYN_REPORT, 0) = %s (%d)\n", strerror(-ret), ret);
    return ret;
  }

  return 0;
}

int main() {
  int ret = 0;
  struct libevdev *evdev_dev = NULL;
  struct libevdev_uinput *uinput_dev = NULL;
  struct udev *udev_ctx = NULL;
  struct udev_monitor *udev_mon = NULL;
  struct sigaction exit_action;

  // Capture SIG{INT,TERM}
  memset(&exit_action, 0, sizeof(struct sigaction));
  exit_action.sa_flags = SA_RESTART;
  exit_action.sa_handler = exit_signal_handler;

  sigaction(SIGINT, &exit_action, NULL);
  sigaction(SIGTERM, &exit_action, NULL);

  // Create evdev device
  evdev_dev = libevdev_new();
  if (!evdev_dev) {
    fprintf(stderr, "libevdev_new() failed.\n");
    goto error;
  }

  // Set evdev device properties
  libevdev_set_name(evdev_dev, "tablet-mode-vswitch");

  // Enable EV_SW event type and SW_TABLET_MODE event code for the device
  ret = libevdev_enable_event_type(evdev_dev, EV_SW);
  if (ret != 0) {
    fprintf(stderr, "libevdev_enable_event_type(evdev_dev, EV_SW) = %s (%d)\n", strerror(-ret), ret);
    goto error;
  }

  ret = libevdev_enable_event_code(evdev_dev, EV_SW, SW_TABLET_MODE, NULL);
  if (ret != 0) {
    fprintf(stderr, "libevdev_enable_event_code(evdev_dev, EV_SW, SW_TABLET_MODE, NULL) = %s (%d)\n", strerror(-ret),
            ret);
    goto error;
  }

  // Create uinput device from the evdev device
  ret = libevdev_uinput_create_from_device(evdev_dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput_dev);
  if (ret != 0) {
    fprintf(stderr,
            "libevdev_uinput_create_from_device(evdev_dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uinput_dev) = %s (%d)\n",
            strerror(-ret), ret);
    goto error;
  }

  // Create udev context
  udev_ctx = udev_new();
  if (!udev_ctx) {
    fprintf(stderr, "udev_new() failed.\n");
    goto error;
  }

  // Monitor udev events over netlink
  udev_mon = udev_monitor_new_from_netlink(udev_ctx, "kernel");
  if (!udev_ctx) {
    fprintf(stderr, "udev_monutor_new_from_netlink(udev, \"udev\") failed.\n");
    goto error;
  }

  // Filter events by hid subsystem
  ret = udev_monitor_filter_add_match_subsystem_devtype(udev_mon, "hid", NULL);
  if (ret != 0) {
    fprintf(stderr, "udev_monitor_filter_add_match_subsystem_devtype(udev_mon, 'hid', NULL) = %s (%d)\n",
            strerror(-ret), ret);
    goto error;
  }

  // Start receiving events
  ret = udev_monitor_enable_receiving(udev_mon);
  if (ret != 0) {
    fprintf(stderr, "udev_monitor_enable_receiving(udev_mon) = %s (%d)\n", strerror(-ret), ret);
    goto error;
  }

  struct pollfd fds = {udev_monitor_get_fd(udev_mon), POLLIN, 0};
  while (exit_signal == 0) {
    if ((poll(&fds, 1, 0) == 1) && (fds.revents & POLLIN)) {
      struct udev_device *dev = udev_monitor_receive_device(udev_mon);
      if (!dev) {
        continue;
      }

      const char *action = udev_device_get_action(dev);
      const char *hid_id = udev_device_get_property_value(dev, "HID_ID");
      const char *hid_name = udev_device_get_property_value(dev, "HID_NAME");

#ifdef DEBUG
      fprintf(stderr, "DEBUG> %s - %s - %s\n", action, hid_id, hid_name);
#endif

      if (strcmp(action, "add") == 0 || strcmp(action, "remove") == 0) {
        for (int idx = 0; hid_ids[idx] != NULL; idx++) {
          if (strcmp(hid_id, hid_ids[idx]) == 0) {
            fprintf(stdout, "%s (%s) %sconnected...\n", hid_name, hid_id, strcmp(action, "add") != 0 ? "dis" : "");
            ret = set_tablet_mode(uinput_dev, strcmp(action, "add"));
            break;
          }
        }
      }

      udev_device_unref(dev);

      if (ret != 0) {
        fprintf(stdout, "stopping due to set tablet mode error...\n");
        break;
      }
    }

    usleep(500 * 1000);
  }

error:
  udev_unref(udev_ctx);
  libevdev_uinput_destroy(uinput_dev);
  libevdev_free(evdev_dev);

  return ret;
}
