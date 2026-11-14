# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../..)
TEST_DEFINES := -DTEST_SDK='"$(ROOT)/src"' -DAMA_EXECUTABLE='"$(ROOT)/ama"' -DTEST_STANDARD_ROOT='"$(ROOT)/stdlib/modules"'
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
