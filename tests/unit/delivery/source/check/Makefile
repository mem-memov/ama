# @file
# @brief Check prepared captured identities through the actual frontend without physical source fallback.
# Contract: ../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/json) \
    $(call source_files,$(ROOT)/src/delivery/source/snapshot) $(call source_files,$(ROOT)/src/delivery/source/domain))
SOURCES += delivery/source/export.c delivery/source/entry.c delivery/source/probe/snapshot.c
SOURCES := $(filter-out delivery/source/snapshot/write.c,$(SOURCES))
include ../../../../leaf.mk
