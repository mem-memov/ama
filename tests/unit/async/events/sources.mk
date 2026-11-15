# @file
# @brief Link the actual native event component without an Ama frontend or execution runtime.
# Contract: ../../../../docs/implementation/native-events.md.
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/events))
LINKER_FLAGS += -pthread -Wl,--wrap=malloc,--wrap=free
