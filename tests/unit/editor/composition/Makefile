# @file
# @brief Check editor composition against actual production frontend and source ownership.
# Contract: ../../../../docs/implementation/editor-analysis.md.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(filter-out compiler/specializations/%,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/compiler)))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/editor))
include ../../../leaf.mk
