# @file
# @brief Compose actual frontend refusal records with the allocation-free human renderer.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/diagnostics/render))
include ../../../leaf.mk
