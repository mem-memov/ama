# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(foreach component,source/identity native/namespace native/status/providers,\
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/$(component)))) \
    source/memory/arena.c source/text/manager.c source/text/lines.c source/text/view.c \
    source/unicode/decode.c source/unicode/identifier.c source/unicode/word.c source/unicode/name.c source/module/resolve/logical.c
TEST_DEFINES = -DTEST_ROOT='"$(ROOT)"' -DTEST_REAL_COMPILER='"$(C_COMPILER)"'
include ../../../leaf.mk
