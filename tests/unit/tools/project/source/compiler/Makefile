# @file
# @brief Verify shell-free compiler collection and exact response admission.
# Contract: ../../../../../../docs/implementation/source-protocol.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := source/text/manager.c source/text/lines.c source/text/view.c source/memory/arena.c \
    source/path/logical.c source/path/version.c source/unicode/decode.c source/unicode/identifier.c \
    source/unicode/word.c source/unicode/name.c delivery/source/wire.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/json) \
    $(call source_files,$(ROOT)/src/source/io) $(call source_files,$(ROOT)/src/delivery/source/snapshot) $(call source_files,$(ROOT)/src/delivery/source/response) \
    $(call source_files,$(ROOT)/src/tools/output/json))
LOCAL_SOURCES := main.c $(ROOT)/tools/project/source/compiler/run.c $(ROOT)/tools/project/source/compiler/lifetime.c $(ROOT)/tools/project/process/run.c
include ../../../../../leaf.mk
