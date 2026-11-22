# @file
# @brief Check original dependency/input deletion safety and report allocation atomicity.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(PROJECT_TOOL_SHARED_SOURCES) source/text/manager.c source/text/lines.c
LOCAL_SOURCES := main.c $(ROOT)/tools/project/source/removal/check.c
include ../../../../../leaf.mk
