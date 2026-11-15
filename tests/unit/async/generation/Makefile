# @file
# @brief Exhaust generation reservation and counted literal conversion independently of a frontend.
# Contract: ../../../../docs/implementation/task-enrollment.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
TEST_DEFINES := -DTEST_ROOT='"$(ROOT)"' -DTEST_C_COMPILER='"$(C_COMPILER)"'
ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
