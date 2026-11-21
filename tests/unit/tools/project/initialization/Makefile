# @file
# @brief Verify real isolated project commands, publication, cancellation and concurrency.
# Contract: ../../../../../docs/implementation/project-tool.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
LOCAL_SOURCES := main.c $(filter-out $(ROOT)/tools/project/main.c,$(call source_files,$(ROOT)/tools/project))
TEST_DEFINES := -DPROJECT_EXECUTABLE='"$(ROOT)/ama-project"'
include $(ROOT)/tools/project/sources.mk
SOURCES := $(PROJECT_TOOL_SHARED_SOURCES)
include ../../../../leaf.mk
.PHONY: project-tool
build: project-tool
project-tool:
>    $(MAKE) -C $(ROOT) ama-project
