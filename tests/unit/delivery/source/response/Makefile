# @file
# @brief Decode independently encoded source-domain responses and reject malformed capture/graph facts.
# Contract: ../../../../../docs/implementation/source-protocol.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := source/text/manager.c source/text/lines.c source/text/view.c source/memory/arena.c \
    source/path/logical.c source/path/version.c source/unicode/decode.c source/unicode/identifier.c \
    source/unicode/word.c source/unicode/name.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/json) \
    $(call source_files,$(ROOT)/src/source/io) $(call source_files,$(ROOT)/src/delivery/source/response))
include ../../../../leaf.mk
