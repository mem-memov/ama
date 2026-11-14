# @file
# @brief Generate the public C ABI and compile independent consumers without compiler SDK includes.
# Contract: ../../../../docs/implementation/c-library-header.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/identity))
SOURCES += compiler/frontend.c compiler/inputs.c compiler/load.c compiler/authoring/check.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
TEST_DEFINES := -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\" -DTEST_C_COMPILER='"$(C_COMPILER)"'
include ../../../leaf.mk
