# @file
# @brief Verify exact read-only relocation discovery across all configured roots.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/project/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation))
include $(ROOT)/tests/leaf.mk
