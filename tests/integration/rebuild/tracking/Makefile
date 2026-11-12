# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../../..)
TEST_DEFINES = -DTEST_ROOT='"$(ROOT)"' -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk

generated/compiler: project/compiler.c generated/compile.configuration
>    $(C_COMPILER) $(CPPFLAGS) $(C_COMPILER_FLAGS) -o $@ $<

generated/test: | generated/compiler
