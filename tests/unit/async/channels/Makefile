# @file
# @brief Verify bounded native FIFO ownership and actual SPSC concurrency with counted resource failures.
# Contract: ../../../../docs/implementation/bounded-channels.md
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/channels))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/messages))
LINKER_FLAGS += -pthread -Wl,--wrap=malloc,--wrap=calloc,--wrap=free
include $(ROOT)/tests/leaf.mk
