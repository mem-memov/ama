# @file
# @brief Verify resolved relocation edges and exact module-prefix edit selection.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/semantic/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation))
include $(ROOT)/tests/leaf.mk
