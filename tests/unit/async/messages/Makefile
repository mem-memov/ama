# @file
# @brief Test data-only native schemas, canonical wire validation and every allocation prefix.
# Contract: ../../../../docs/implementation/typed-messages.md
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/messages))
LINKER_FLAGS += -Wl,--wrap=malloc,--wrap=free
include $(ROOT)/tests/leaf.mk
