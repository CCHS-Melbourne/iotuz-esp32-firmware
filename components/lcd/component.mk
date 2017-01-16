#
# Component Makefile
#
COMPONENT_ADD_INCLUDEDIRS := Adafruit_ILI9341 Adafruit-GFX iotuz_graphics
COMPONENT_ADD_INCLUDEDIRS += ../../main
#COMPONENT_PRIV_INCLUDEDIRS :=

COMPONENT_SRCDIRS := Adafruit_ILI9341 Adafruit-GFX iotuz_graphics

# using arduino libraries post 1.0
CPPFLAGS += -DARDUINO=101 -DESP32 -Wno-error=maybe-uninitialized
