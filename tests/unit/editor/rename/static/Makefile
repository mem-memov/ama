# @file
# @brief Check source static names and every captured-owner rename correspondence against the production frontend.
# Contract: ../../../../../docs/implementation/editor-analysis.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/compiler))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/editor))
TEST_DEFINES := -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\"
include ../../../../leaf.mk
