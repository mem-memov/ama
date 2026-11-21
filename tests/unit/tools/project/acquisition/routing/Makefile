# @file
# @brief Compose actual strict INI scopes with exact repository route selection.
# Contract: ../../../../../../docs/implementation/acquisition-plan.md.
ROOT := $(abspath ../../../../../..)
include ../sources.mk
LOCAL_SOURCES := main.c $(ROOT)/tools/project/acquisition/routing/select.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/configuration/aliases))
include ../../../../../leaf.mk
