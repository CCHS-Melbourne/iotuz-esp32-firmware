# esp32-iotuz

This is the firmware for the iotuz board being developed for linuxconf 2017.

# building

After cloning this repository remember to update submodules.

```
git submodule update --init
```

Use the menu to configure your Wifi and MQTT server.

```
make menuconfig
```

To build and flash.

```
make
ESPPORT=/dev/tty.SLAB_USBtoUART make flash
```

# todo

* [ ] Add the event loop for buttons on the ioextender send to MQTT and local get value.
* [ ]

# references

* [Espressif IoT Development Framework](https://github.com/espressif/esp-idf)
* [esp32-mqtt](https://github.com/tuanpmt/esp32-mqtt)
