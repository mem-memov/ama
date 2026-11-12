# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../../..)
SOURCES := native/toolchain/symbols.c native/toolchain/listing/record.c native/toolchain/listing/match.c
TEST_DEFINES = -DTEST_ROOT='"$(ROOT)"' -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk
