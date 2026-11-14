# @file
# @brief Exercise generated typed input recipes and independent retained result snapshots.
# Contract: ../../../../docs/implementation/c-library-transport.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
TEST_DEFINES := -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\" -DTEST_SDK=\"$(ROOT)/src\" -DTEST_C_COMPILER='"$(C_COMPILER)"'
ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include ../../../leaf.mk
