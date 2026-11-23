# @file
# @brief Verify lossless durable image facts, closed schema and failure ownership through real streams.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES) $(PROJECT_TOOL_RECORD_SOURCES))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/image) \
    $(call source_files,$(ROOT)/tools/project/source/transaction/document) \
    $(ROOT)/tools/project/source/observation/freshness.c
include ../../../../../leaf.mk
