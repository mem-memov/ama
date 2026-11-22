# @file
# @brief Verify physical compiler-input capture against independent files and actual compiler admission.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
SOURCES += delivery/source/snapshot/write.c delivery/source/wire.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/output/json))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/capture) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure/unit) \
    $(call source_files,$(ROOT)/tools/project/source/compiler) $(ROOT)/tools/project/process/run.c
include ../../../../../leaf.mk
