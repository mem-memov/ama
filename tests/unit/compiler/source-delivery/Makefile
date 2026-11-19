# @file
# @brief Compare distributed source snapshots and render exact loader/specialization refusals.
# Contract: ../../../../docs/implementation/template-source-delivery.md.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/diagnostics/render))
include ../../../leaf.mk
