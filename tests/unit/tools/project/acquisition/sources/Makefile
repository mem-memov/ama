# @file
# @brief Verify operation-local snapshot/source contracts and actual prepared-version precedence.
# Contract: ../../../../../../docs/implementation/acquisition-plan.md.
ROOT := $(abspath ../../../../../..)
include ../sources.mk
LOCAL_SOURCES := main.c $(ROOT)/tools/project/acquisition/routing/select.c \
    $(call source_files,$(ROOT)/tools/project/acquisition/location) \
    $(call source_files,$(ROOT)/tools/project/acquisition/sources)
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/module)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/io)) source/path/exact.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/configuration/aliases) \
    $(call source_files,$(ROOT)/src/source/configuration/selectors) $(call source_files,$(ROOT)/src/source/configuration/selection))
SOURCES := $(sort $(SOURCES))
include ../../../../../leaf.mk
