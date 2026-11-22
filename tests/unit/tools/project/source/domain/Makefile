# @file
# @brief Compose complete captured source-domain analysis with real parser and semantic checking.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += delivery/source/export.c delivery/source/domain/required.c delivery/source/domain/check.c delivery/source/domain/native.c
LOCAL_SOURCES := main.c $(ROOT)/tools/project/source/removal/check.c
include ../../../../../leaf.mk
