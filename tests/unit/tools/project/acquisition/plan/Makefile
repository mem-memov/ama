# @file
# @brief Compose finite closure control with real prepared files and checked compiler exports.
# Contract: ../../../../../../docs/implementation/acquisition-plan.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += delivery/source/entry.c delivery/source/export.c delivery/native/files/capture.c
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/acquisition/closure) \
    $(call source_files,$(ROOT)/tools/project/acquisition/sources) \
    $(call source_files,$(ROOT)/tools/project/acquisition/routing) \
    $(call source_files,$(ROOT)/tools/project/acquisition/location)
include ../../../../../leaf.mk
