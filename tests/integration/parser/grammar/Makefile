# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(foreach component,source diagnostics lexer ast parser native/namespace,\
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/$(component))))
TEST_DEFINES = -DTEST_REAL_COMPILER='"$(C_COMPILER)"' -DTEST_ROOT='"$(ROOT)"'
include $(ROOT)/tests/leaf.mk
