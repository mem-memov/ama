# @file
# @brief Verify owned compiler specialization stages through real source, scope and required-Type components.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/semantic/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/compiler/specializations))
include ../../../leaf.mk
