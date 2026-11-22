# @file
# @brief Check concrete native-origin selection without executing native companions.
# Contract: ../../../../docs/implementation/build-launcher-contracts.md.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/delivery))
include ../../../leaf.mk
