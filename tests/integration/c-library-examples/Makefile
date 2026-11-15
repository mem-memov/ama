# @file
# @brief Exercise the published multi-file C hosts through the actual library-build CLI.
# Contract: ../../../docs/implementation/c-library-examples.md.
ROOT := $(abspath ../../..)
C_COMPILER ?= gcc
TEST_DEFINES := -DTEST_C_COMPILER='"$(C_COMPILER)"' -DTEST_EXAMPLES='"$(ROOT)/tools/examples/c_library"' -DAMA_EXECUTABLE='"$(ROOT)/ama"'
ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
