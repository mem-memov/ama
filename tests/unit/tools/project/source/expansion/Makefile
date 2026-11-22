# @file
# @brief Compose physical captures and real compiler analysis with structural closure preparation.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
SOURCES += delivery/source/snapshot/write.c delivery/source/wire.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/output/json))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/observation) \
    $(call source_files,$(ROOT)/tools/project/source/image) \
    $(call source_files,$(ROOT)/tools/project/source/expansion) \
    $(call source_files,$(ROOT)/tools/project/source/capture) \
    $(call source_files,$(ROOT)/tools/project/source/inputs) \
    $(call source_files,$(ROOT)/tools/project/source/analysis) \
    $(call source_files,$(ROOT)/tools/project/source/compiler) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure) \
    $(call source_files,$(ROOT)/tools/project/source/edits) \
    $(call source_files,$(ROOT)/tools/project/source/operands) \
    $(addprefix $(ROOT)/tools/project/source/mapping/,add.c lifetime.c seal.c) \
    $(call source_files,$(ROOT)/tools/project/source/graph) $(ROOT)/tools/project/process/run.c
include ../../../../../leaf.mk
