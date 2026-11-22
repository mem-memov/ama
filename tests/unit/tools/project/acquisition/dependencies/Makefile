# @file
# @brief Verify actual compiler edges feed finite delivery roles with original provenance.
# Contract: ../../../../../../docs/implementation/acquisition-plan.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += delivery/source/export.c
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/acquisition/closure/roles) \
    $(ROOT)/tools/project/acquisition/closure/dependencies/expand.c
include ../../../../../leaf.mk
