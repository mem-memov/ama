# @file
# @brief Compose parsed source commands with actual compiler requests and the complete operation certificate.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(PROJECT_TOOL_ALL_SHARED_SOURCES)
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/command/source) \
    $(call source_files,$(ROOT)/tools/project/source) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure) $(ROOT)/tools/project/process/run.c
include ../../../../../leaf.mk
