# @file
# @brief Compose a finite prepared image with the actual common complete catalog and measured zero disk I/O.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/image) \
    $(ROOT)/tests/unit/source/module/packages/support/io.c
LINKER_FLAGS += -Wl,--wrap=opendir -Wl,--wrap=readdir -Wl,--wrap=closedir -Wl,--wrap=lstat
include ../../../../../leaf.mk
