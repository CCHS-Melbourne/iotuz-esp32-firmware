#
# Component Makefile
#
COMPONENT_ADD_INCLUDEDIRS := include
#COMPONENT_PRIV_INCLUDEDIRS :=

COMPONENT_SRCDIRS :=  .
#EXTRA_CFLAGS := -DICACHE_RODATA_ATTR
CFLAGS += -Wno-error=implicit-function-declaration -Wno-error=format= -DHAVE_CONFIG_H
