# @file
# @brief Verify exact read-only relocation discovery across all configured roots.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/project/sources.mk
SOURCES += $(foreach component,preview paths roots discovery,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/$(component))))
include $(ROOT)/tests/leaf.mk
