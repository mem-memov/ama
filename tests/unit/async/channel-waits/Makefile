# @file
# @brief Verify exact channel readiness registration/cancellation against real native event services.
# Contract: ../../../../docs/implementation/channel-waits.md
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/channels))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/messages))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/events))
LINKER_FLAGS += -pthread -Wl,--wrap=malloc,--wrap=calloc,--wrap=free
include $(ROOT)/tests/leaf.mk
