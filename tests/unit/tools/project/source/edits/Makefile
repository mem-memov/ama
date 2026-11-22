# @file
# @brief Verify exact parsed-span preparation, boundary provenance and allocation failures.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := source/memory/arena.c source/text/manager.c source/text/lines.c source/text/view.c
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/source/edits)
include ../../../../../leaf.mk
