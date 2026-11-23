# @file
# @brief Verify isolated prepared jobs and exact subprocess observations.
# Contract: ../../../../../docs/implementation/project-tool.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
LOCAL_SOURCES := main.c $(filter-out $(ROOT)/tools/project/main.c,$(call source_files,$(ROOT)/tools/project))
SOURCES := $(PROJECT_TOOL_ALL_SHARED_SOURCES)
include ../../../../leaf.mk
