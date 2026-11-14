# @file
# @brief Link an independent C consumer to generated component lifecycle implementations.
# Contract: ../../../../docs/implementation/c-library-lifecycle.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
TEST_DEFINES := -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\" -DTEST_SDK=\"$(ROOT)/src\" -DTEST_C_COMPILER='"$(C_COMPILER)"'
include ../../../leaf.mk
