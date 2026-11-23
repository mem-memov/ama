# @file
# @brief Check hierarchical directory leases, exact root/ancestor identity and complete failure retirement.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/image) \
    $(call source_files,$(ROOT)/tools/project/source/transaction/lease)
include ../../../../../leaf.mk
