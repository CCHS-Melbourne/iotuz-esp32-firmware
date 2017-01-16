#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := app-template

export IDF_PATH := $(abspath ./esp-idf)

PROJECT_MK := $(IDF_PATH)/make/project.mk

ifeq ($(wildcard $(PROJECT_MK)),)
$(error esp-idf submodule is missing. Run 'git submodule update --init --recursive')
endif

include $(PROJECT_MK)
