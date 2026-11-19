# @file
# @brief Generate the public C ABI and compile independent consumers without compiler SDK includes.
# Contract: ../../../../docs/implementation/c-library-header.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
TEST_DEFINES := -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\" -DTEST_C_COMPILER='"$(C_COMPILER)"'
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
include ../../../leaf.mk
