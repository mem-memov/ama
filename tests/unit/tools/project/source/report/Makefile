# @file
# @brief Decode real compiler-domain transport independently and exercise borrowed-stream failures.
# Contract: ../../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += delivery/source/export.c delivery/source/domain/required.c delivery/source/domain/check.c delivery/source/domain/native.c \
    delivery/source/wire.c delivery/source/report/catalog.c delivery/source/report/graph.c delivery/source/report/write.c \
    tools/output/json/stream.c tools/output/json/text.c tools/output/json/values.c
LOCAL_SOURCES := main.c
include ../../../../../leaf.mk
