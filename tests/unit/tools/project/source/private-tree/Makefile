# @file
# @brief Verify actual private staged trees, complete readback and safe failure cleanup.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/image) \
    $(call source_files,$(ROOT)/tools/project/source/transaction/forest) \
    $(call source_files,$(ROOT)/tools/project/source/transaction/tree)
include ../../../../../leaf.mk
