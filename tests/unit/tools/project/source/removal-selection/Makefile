# @file
# @brief Compose actual source analysis, physical selection and original removal graph/client reviews.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
SOURCES += delivery/source/snapshot/write.c delivery/source/wire.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/output/json))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/removal) \
    $(call source_files,$(ROOT)/tools/project/source/selection) \
    $(call source_files,$(ROOT)/tools/project/source/inputs) \
    $(call source_files,$(ROOT)/tools/project/source/capture) \
    $(call source_files,$(ROOT)/tools/project/source/analysis) \
    $(call source_files,$(ROOT)/tools/project/source/compiler) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure/unit) \
    $(ROOT)/tools/project/acquisition/closure/tests/association.c \
    $(ROOT)/tools/project/acquisition/closure/tests/namespace.c \
    $(ROOT)/tools/project/source/graph/equal.c $(ROOT)/tools/project/process/run.c
include ../../../../../leaf.mk
