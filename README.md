# iotuz-esp32-firmware

This is the firmware for the IoTuz board being developed for Linux Conf 2017.

IoTuz hardware can be found at https://github.com/CCHS-Melbourne/iotuz-esp32-hardware

# Building

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

# References

* [Espressif IoT Development Framework](https://github.com/espressif/esp-idf)
* [esp32-mqtt](https://github.com/tuanpmt/esp32-mqtt)
