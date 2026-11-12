# @file
# @brief Build and run this leaf against its explicit production and fixture sources.
# Contract: ../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += compiler/frontend.c compiler/load.c compiler/authoring/check.c compiler/inspect/module.c
include ../../../leaf.mk
