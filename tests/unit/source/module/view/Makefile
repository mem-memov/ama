# @file
# @brief Verify complete reader-driven catalog discovery and lazy companions without disk reads.
# Contract: ../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
LOCAL_SOURCES := main.c ../packages/support/io.c
LINKER_FLAGS += -Wl,--wrap=opendir -Wl,--wrap=readdir -Wl,--wrap=closedir -Wl,--wrap=lstat
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/module) \
    $(call source_files,$(ROOT)/src/source/io))
SOURCES += source/text/manager.c source/text/lines.c source/text/view.c source/memory/arena.c \
    source/unicode/decode.c source/unicode/identifier.c source/unicode/word.c source/unicode/name.c \
    source/path/exact.c source/path/logical.c source/path/version.c
include ../../../../leaf.mk
