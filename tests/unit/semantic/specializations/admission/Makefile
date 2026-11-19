# @file
# @brief Verify actual ordinary specialization admission and complete resource refusal prefixes.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/semantic/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/compiler/specializations))
include ../../../../leaf.mk
