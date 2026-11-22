# @file
# @brief Verify byte-complete module units, context exclusions and filesystem refusals.
# Contract: ../../../../../../docs/implementation/acquisition-plan.md.
ROOT := $(abspath ../../../../../..)
include ../sources.mk
SOURCES := $(sort $(SOURCES) source/path/exact.c $(patsubst $(ROOT)/src/%,%,\
    $(call source_files,$(ROOT)/src/source/module) $(call source_files,$(ROOT)/src/source/io)))
LOCAL_SOURCES := main.c $(call source_files,$(ROOT)/tools/project/acquisition/closure/unit) \
    $(ROOT)/src/delivery/native/files/capture.c
include ../../../../../leaf.mk
