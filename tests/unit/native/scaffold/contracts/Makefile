# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/native/scaffold/sources.mk
TEST_DEFINES = -DTEST_SDK=\"$(ROOT)/src\" -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk
