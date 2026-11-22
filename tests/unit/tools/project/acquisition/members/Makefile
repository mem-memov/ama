# @file
# @brief Verify unique captured memberships and persistent inclusion/snapshot evidence.
# Contract: ../../../../../../docs/implementation/acquisition-plan.md.
ROOT := $(abspath ../../../../../..)
include ../sources.mk
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/acquisition/closure/members) \
    $(call source_files,$(ROOT)/tools/project/acquisition/closure/roles)
include ../../../../../leaf.mk
