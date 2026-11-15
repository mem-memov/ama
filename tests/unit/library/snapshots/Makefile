# @file
# @brief Verify independent public C snapshot ownership and every partial-clone allocation failure.
# Contract: ../../../../docs/implementation/c-library-transport.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/identity))
SOURCES += compiler/frontend.c compiler/inputs.c compiler/load.c compiler/authoring/check.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
TEST_DEFINES := -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\" -DTEST_SDK=\"$(ROOT)/src\" -DTEST_C_COMPILER='"$(C_COMPILER)"'
ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
include ../../../leaf.mk
