# @file
# @brief Verify real protected private request files and safe partial/foreign cleanup.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(PROJECT_TOOL_ALL_SHARED_SOURCES)
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/workspace) \
    $(call source_files,$(ROOT)/tools/project/source/capture) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure/unit) \
    $(call source_files,$(ROOT)/tools/project/source/image) \
    $(call source_files,$(ROOT)/tools/project/source/transaction/tree)
include ../../../../../leaf.mk
