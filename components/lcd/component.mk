#
# Component Makefile
#
# This Makefile should, at the very least, just include $(SDK_PATH)/make/component.mk. By default,
# this will take the sources in this directory, compile them and link them into
# lib(subdirectory_name).a in the build directory. This behaviour is entirely configurable,
# please read the SDK documents if you need to do this.
#
COMPONENT_ADD_INCLUDEDIRS := Adafruit_ILI9341 Adafruit-GFX
#COMPONENT_PRIV_INCLUDEDIRS :=

COMPONENT_SRCDIRS := Adafruit_ILI9341 Adafruit-GFX

# using arduino libraries post 1.0
CPPFLAGS += -DARDUINO=101 -DESP32 -Wno-error=maybe-uninitialized



include $(IDF_PATH)/make/component_common.mk
