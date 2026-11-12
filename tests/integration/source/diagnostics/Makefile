# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/diagnostics))
TEST_DEFINES = -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include $(ROOT)/tests/leaf.mk
