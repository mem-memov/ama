# @file
# @brief Compile and execute two ordinary Ama microprograms through isolated test jobs.
# Contract: ../../../../../docs/implementation/project-tool.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
LOCAL_SOURCES := main.c $(filter-out $(ROOT)/tools/project/main.c,$(call source_files,$(ROOT)/tools/project))
SOURCES := $(PROJECT_TOOL_SHARED_SOURCES)
TEST_DEFINES := -DAMA_EXECUTABLE='"$(ROOT)/ama"' -DTEST_SDK='"$(ROOT)/src"' -DTEST_PROJECT='"$(CURDIR)/project"'
include ../../../../leaf.mk
.PHONY: compiler
build: compiler
compiler:
>    $(MAKE) -C $(ROOT) all
