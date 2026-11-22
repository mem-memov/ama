# @file
# @brief Verify common final image discovery and complete owned compiler snapshots with zero source-disk I/O.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES) $(PROJECT_TOOL_CAPTURE_SOURCES))
SOURCES += delivery/source/snapshot/write.c delivery/source/wire.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/output/json))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/capture) \
    $(call source_files,$(ROOT)/tools/project/source/image) \
    $(call source_files,$(ROOT)/tools/project/source/compiler) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure/unit) \
    $(ROOT)/tools/project/process/run.c $(ROOT)/tests/unit/source/module/packages/support/io.c
LINKER_FLAGS += -Wl,--wrap=opendir -Wl,--wrap=readdir -Wl,--wrap=closedir -Wl,--wrap=lstat
include ../../../../../leaf.mk
