#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := app-template

include $(IDF_PATH)/make/project.mk

monitor:
	miniterm.py --dtr 0 --rts 0 --raw ${ESPPORT} 115200
