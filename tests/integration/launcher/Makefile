# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../..)
TEST_DEFINES := -DAMA_EXECUTABLE='"$(ROOT)/ama"'
include $(ROOT)/tests/leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

generated/test: | compiler
