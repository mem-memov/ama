# @file
# @brief Compose actual raw selection, compiler proof, structural facts and explicit copy dependency modes.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
SOURCES += delivery/source/snapshot/write.c delivery/source/wire.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/output/json))
LOCAL_SOURCES := main.c $(ROOT)/tools/project/source/removal/check.c $(ROOT)/tools/project/source/removal/roots.c \
    $(call source_files,$(ROOT)/tools/project/source/removal/selection) $(call source_files,$(ROOT)/tools/project/source/image) \
    $(call source_files,$(ROOT)/tools/project/source/environment) \
    $(call source_files,$(ROOT)/tools/project/source/observation) \
    $(call source_files,$(ROOT)/tools/project/source/edits) \
    $(call source_files,$(ROOT)/tools/project/source/operands) $(call source_files,$(ROOT)/tools/project/source/mapping) $(call source_files,$(ROOT)/tools/project/source/copy) \
    $(call source_files,$(ROOT)/tools/project/source/expansion) \
    $(call source_files,$(ROOT)/tools/project/source/inputs) \
    $(call source_files,$(ROOT)/tools/project/source/capture) \
    $(call source_files,$(ROOT)/tools/project/source/selection) \
    $(call source_files,$(ROOT)/tools/project/source/analysis) \
    $(call source_files,$(ROOT)/tools/project/source/compiler) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure) \
    $(ROOT)/tools/project/source/graph/bindings.c $(ROOT)/tools/project/source/graph/sites.c \
    $(ROOT)/tools/project/source/graph/equal.c $(ROOT)/tools/project/process/run.c
include ../../../../../leaf.mk
