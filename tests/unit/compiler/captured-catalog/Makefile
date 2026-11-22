# @file
# @brief Verify captured compiler catalogs without executable entries or future-path filesystem probes.
# Contract: ../../../../docs/implementation/static-relocation.md.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += delivery/source/entry.c
include $(ROOT)/tests/leaf.mk
