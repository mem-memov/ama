# @file
# @brief Check actual standalone incremental builds, flag invalidation and link membership.
# Contract: ../../../../../docs/implementation/project-tool.md.
ROOT := $(abspath ../../../../..)
LOCAL_SOURCES := main.c $(ROOT)/tools/project/process/run.c
TEST_DEFINES := -DPROJECT_MAKE_DIRECTORY='"$(ROOT)/tools/project"'
include ../../../../leaf.mk
