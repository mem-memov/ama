# @file
# @brief Verify injective nominal/physical clone proposals over actual original catalog handles.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
LOCAL_SOURCES := main.c $(ROOT)/tools/project/acquisition/closure/tests/namespace.c $(call source_files,$(ROOT)/tools/project/source/mapping) \
    $(call source_files,$(ROOT)/tools/project/source/capture) \
    $(call source_files,$(ROOT)/tools/project/source/selection) \
    $(ROOT)/tools/project/source/graph/bindings.c \
    $(ROOT)/tools/project/source/graph/sites.c \
    $(call source_files,$(ROOT)/tools/project/source/edits) \
    $(call source_files,$(ROOT)/tools/project/source/operands) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure/unit)
include ../../../../../leaf.mk
