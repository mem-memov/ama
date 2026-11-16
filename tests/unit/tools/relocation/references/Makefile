# @file
# @brief Verify resolved relocation edges and exact module-prefix edit selection.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/semantic/sources.mk
SOURCES += $(foreach component,preview paths roots discovery references,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/$(component))))
include $(ROOT)/tests/leaf.mk
