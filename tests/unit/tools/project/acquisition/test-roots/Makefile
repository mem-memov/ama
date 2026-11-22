# @file
# @brief Compose direct test-root selection with actual definition-time frontend checking.
# Contract: ../../../../../../docs/implementation/acquisition-plan.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += delivery/source/entry.c delivery/source/export.c
LOCAL_SOURCES := main.c $(ROOT)/tools/project/acquisition/closure/tests/roots.c
include ../../../../../leaf.mk
