# @file
# @brief Verify original source analysis and cached T235 admission through controlled and actual compiler processes.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
SOURCES += $(filter-out delivery/source/snapshot/write.c,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/delivery/source/snapshot)))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/analysis) \
    $(call source_files,$(ROOT)/tools/project/source/compiler) $(ROOT)/tools/project/source/graph/equal.c \
    $(ROOT)/tools/project/acquisition/closure/tests/roots.c $(ROOT)/tools/project/process/run.c
include ../../../../../leaf.mk
