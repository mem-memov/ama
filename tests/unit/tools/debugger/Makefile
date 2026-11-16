# @file
# @brief Exercise exact debugger commands, real admitted context observations and failure cleanup.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/runtime/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/debugger))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/output/json))
SOURCES += source/unicode/decode.c
include $(ROOT)/tests/leaf.mk
