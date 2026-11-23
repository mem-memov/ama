# @file
# @brief Verify initializer composition and the real tool without linking unrelated project planners into its unit harness.
# Contract: ../../../../../docs/implementation/project-tool.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
LOCAL_SOURCES := main.c $(ROOT)/tools/project/command/request.c $(ROOT)/tools/project/command/run.c \
    $(call source_files,$(ROOT)/tools/project/init)
TEST_DEFINES := -DPROJECT_EXECUTABLE='"$(ROOT)/ama-project"'
include ../../../../leaf.mk
.PHONY: project-tool
build: project-tool
project-tool:
>    $(MAKE) -C $(ROOT) ama-project
