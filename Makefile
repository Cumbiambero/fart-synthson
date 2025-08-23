RACK_DIR ?= ../..

FLAGS += -isystem $(RACK_DIR)/include
FLAGS += -Wpedantic -Wconversion -Wno-psabi
CXXFLAGS += -std=c++23

SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

include $(RACK_DIR)/plugin.mk

CXXFLAGS := $(filter-out -std=c++11,$(CXXFLAGS))
